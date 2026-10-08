/**
 * Two-pass assembler for the 6502 emulator.
 *
 * Each source line goes through:
 *
 *   lexLine()    splits the line into typed tokens (numbers, identifiers,
 *                directives, '#'), dropping comments and recording columns
 *   parseLine()  turns the tokens into a Statement: an optional location
 *                ($addr at the start of the line), an optional label (an
 *                identifier in column 1), then an instruction with its
 *                operand or a .DATA list
 *
 * Pass 1 is the parse: every statement is given an address and every label
 * is recorded. Pass 2 (emit()) writes the bytes, which is when label
 * operands are resolved, so forward references work.
 *
 * Syntax accepted (see docs/ASSEMBLER_PLAN.md for where this is going):
 *
 *   [$addr] [label] MNEMONIC [operand]   ; comment
 *   [$addr] [label] .DATA value ...      ; comment
 *
 * The addressing mode is spelled in the mnemonic (LDAI, LDAZ, STAA, ...).
 * Operands are #$hh or #ddd (immediate), $hh or $hhhh (address), or a label.
 * .DATA values are hex, with or without a '$' prefix; a value above $FF is
 * stored as two bytes, low byte first. Source is case-insensitive.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>

#include "asm6502.h"
#include "ftrace.h"

static const int kMaxLineLength = 1024;

namespace
{

#define LOBYTE(w) ((uint8_t)((w) & 0xff))
#define HIBYTE(w) ((uint8_t)(((w) >> 8) & 0xff))

/**
 * One word of a source line, classified by its first character.
 */
struct Token
{
    enum Kind
    {
        kNumber,     // $hex or decimal digits
        kIdentifier, // mnemonic or label
        kDirective,  // .DATA
        kHash        // '#', introduces an immediate value
    };

    Kind          kind;
    std::string   text;   // source text, uppercased, without any '$' prefix
    bool          hex;    // kNumber: had a '$' prefix
    unsigned long value;  // kNumber: numeric value
    int           col;    // 1-based column of the first character
};

/**
 * An instruction's operand.
 */
struct Operand
{
    enum Kind
    {
        kNone,
        kImmediate, // #value, one byte
        kAddress,   // $addr literal, one or two bytes
        kLabel      // symbolic, resolved in pass 2
    };

    Kind        kind;
    uint16_t    value;
    uint8_t     width; // bytes occupied; for kLabel decided by the instruction
    std::string label;
    int         col;

    Operand() : kind(kNone), value(0), width(0), col(0) {}
};

/**
 * One assembled item: an instruction or a run of data bytes.
 */
struct Statement
{
    enum Kind
    {
        kInstruction,
        kData
    };

    Kind                 kind;
    uint16_t             address;
    int                  line;
    uint8_t              opcode;  // kInstruction
    Operand              operand; // kInstruction
    std::vector<uint8_t> data;    // kData
};

/**
 * The eight relative branch instructions share the low five opcode bits.
 */
bool isBranch(uint8_t opcode)
{
    return (opcode & 0x1f) == 0x10;
}

class Assembler
{
public:
    Assembler(const char* filename, uint8_t* memory);

    /**
     * Assemble the whole file. Returns the error count, or -1 if the file
     * could not be opened.
     */
    int run();

private:
    // Pass 1
    bool lexLine(const char* line, int lineno, std::vector<Token>& tokens);
    void parseLine(const std::vector<Token>& tokens, int lineno);
    bool parseOperand(const std::vector<Token>& tokens, size_t first,
                      uint8_t opcode, int lineno, Operand& operand);
    bool parseData(const std::vector<Token>& tokens, size_t first,
                   int lineno, std::vector<uint8_t>& data);

    // Pass 2
    void emit();

    void error(int line, int col, const char* fmt, ...);

    const char*                     filename_;
    uint8_t*                        memory_;
    uint16_t                        pc_;      // location counter
    int                             errors_;
    std::map<std::string, uint16_t> labels_;
    std::vector<Statement>          statements_;
};

Assembler::Assembler(const char* filename, uint8_t* memory)
    : filename_(filename), memory_(memory), pc_(0), errors_(0)
{
    assert(filename);
    assert(memory);
}

/**
 * Report an error. Assembly continues so every error in the file is seen.
 */
void Assembler::error(int line, int col, const char* fmt, ...)
{
    char message[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    fprintf(stderr, "%s:%d:%d: error: %s\n", filename_, line, col, message);
    errors_++;
}

/**
 * Split a line into tokens. Comments (';' to end of line) are dropped.
 * Returns false after reporting an error.
 */
bool Assembler::lexLine(const char* line, int lineno, std::vector<Token>& tokens)
{
    size_t i = 0;
    size_t len = strlen(line);

    while (i < len)
    {
        char c = line[i];

        if (isspace((unsigned char)c))
        {
            i++;
            continue;
        }

        if (c == ';') break; // comment

        Token token;
        token.col = (int)i + 1;
        token.hex = false;
        token.value = 0;

        if (c == '#')
        {
            token.kind = Token::kHash;
            token.text = "#";
            i++;
        }
        else if (c == '$' || isdigit((unsigned char)c))
        {
            token.kind = Token::kNumber;
            token.hex = (c == '$');
            if (token.hex) i++;

            size_t start = i;
            while (i < len && isalnum((unsigned char)line[i]))
            {
                token.text += (char)toupper((unsigned char)line[i]);
                i++;
            }

            const char* digits = token.hex ? "0123456789ABCDEF" : "0123456789";
            if (token.text.empty() ||
                strspn(token.text.c_str(), digits) != token.text.size())
            {
                error(lineno, (int)start + 1, "bad %s value ->%s%s<-",
                      token.hex ? "hex" : "decimal", token.hex ? "$" : "",
                      token.text.c_str());
                return false;
            }

            // Digit count is validated by the parser, so cap the value here
            // rather than overflow on an absurdly long literal.
            if (token.text.size() <= 8)
            {
                token.value = strtoul(token.text.c_str(), NULL, token.hex ? 16 : 10);
            }
            else
            {
                token.value = 0xffffffff;
            }
        }
        else if (c == '.' || isalpha((unsigned char)c) || c == '_')
        {
            token.kind = (c == '.') ? Token::kDirective : Token::kIdentifier;
            while (i < len && (isalnum((unsigned char)line[i]) ||
                               line[i] == '_' || line[i] == '.'))
            {
                token.text += (char)toupper((unsigned char)line[i]);
                i++;
            }
        }
        else
        {
            error(lineno, (int)i + 1, "unexpected character '%c'", c);
            return false;
        }

        tokens.push_back(token);
    }

    return true;
}

/**
 * Parse the operand tokens for the instruction with the given opcode.
 * Returns false after reporting an error.
 */
bool Assembler::parseOperand(const std::vector<Token>& tokens, size_t first,
                             uint8_t opcode, int lineno, Operand& operand)
{
    uint8_t bytes = asmInstructionBytes(opcode);
    size_t  count = tokens.size() - first;

    if (count == 0)
    {
        if (bytes > 1)
        {
            error(lineno, tokens.back().col + (int)tokens.back().text.size(),
                  "missing operand");
            return false;
        }
        return true;
    }

    if (bytes == 1)
    {
        error(lineno, tokens[first].col, "instruction takes no operand");
        return false;
    }

    const Token& tok = tokens[first];
    operand.col = tok.col;

    if (tok.kind == Token::kHash) // immediate value
    {
        if (count < 2 || tokens[first+1].kind != Token::kNumber)
        {
            error(lineno, tok.col, "expected a value after #");
            return false;
        }

        const Token& num = tokens[first+1];

        if (num.hex && num.text.size() > 2)
        {
            error(lineno, num.col, "wrong number of digits in hex value, ->$%s<-",
                  num.text.c_str());
            return false;
        }
        if (!num.hex && num.text.size() > 3)
        {
            error(lineno, num.col, "wrong number of digits in decimal value, ->%s<-",
                  num.text.c_str());
            return false;
        }
        if (num.value > 0xff)
        {
            error(lineno, num.col, "immediate value out of range, ->%s<-",
                  num.text.c_str());
            return false;
        }

        operand.kind = Operand::kImmediate;
        operand.value = (uint16_t)num.value;
        operand.width = 1;
        count -= 2;
        first += 2;
    }
    else if (tok.kind == Token::kNumber) // address
    {
        if (!tok.hex)
        {
            error(lineno, tok.col, "address must be hex, ->%s<-", tok.text.c_str());
            return false;
        }
        if (tok.text.size() > 4)
        {
            error(lineno, tok.col, "wrong number of digits in hex value, ->$%s<-",
                  tok.text.c_str());
            return false;
        }

        // A 3-byte instruction needs all four digits; a 2-byte one needs a
        // value that fits in a byte and is not written with four digits.
        bool wide = (tok.text.size() == 4 || tok.value > 0xff);

        if (bytes == 3 && tok.text.size() != 4)
        {
            error(lineno, tok.col,
                  "3-byte instruction requires 4-digit hex address, got ->$%s<-",
                  tok.text.c_str());
            return false;
        }
        if (bytes == 2 && wide)
        {
            error(lineno, tok.col,
                  "2-byte instruction requires 1-byte address, got ->$%s<-",
                  tok.text.c_str());
            return false;
        }

        operand.kind = Operand::kAddress;
        operand.value = (uint16_t)tok.value;
        operand.width = bytes - 1;
        count--;
        first++;
    }
    else if (tok.kind == Token::kIdentifier) // label, resolved in pass 2
    {
        operand.kind = Operand::kLabel;
        operand.label = tok.text;
        operand.width = bytes - 1;
        count--;
        first++;
    }
    else
    {
        error(lineno, tok.col, "unexpected token ->%s<-", tok.text.c_str());
        return false;
    }

    if (count > 0)
    {
        error(lineno, tokens[first].col, "unexpected token after operand ->%s<-",
              tokens[first].text.c_str());
        return false;
    }

    return true;
}

/**
 * Parse the values of a .DATA directive. Each is 1-4 hex digits with an
 * optional '$' prefix; values above $FF take two bytes, low byte first.
 */
bool Assembler::parseData(const std::vector<Token>& tokens, size_t first,
                          int lineno, std::vector<uint8_t>& data)
{
    if (first >= tokens.size())
    {
        error(lineno, tokens.back().col, ".DATA requires at least one value");
        return false;
    }

    for (size_t i = first; i < tokens.size(); i++)
    {
        const Token& tok = tokens[i];

        // Bare hex such as AB lexes as an identifier and 55 as a decimal
        // number; .DATA treats both as hex digits.
        bool ok = (tok.kind == Token::kNumber || tok.kind == Token::kIdentifier) &&
                  !tok.text.empty() && tok.text.size() <= 4 &&
                  strspn(tok.text.c_str(), "0123456789ABCDEF") == tok.text.size();

        if (!ok)
        {
            error(lineno, tok.col, "invalid hex value in data section, ->%s%s<-",
                  tok.hex ? "$" : "", tok.text.c_str());
            return false;
        }

        uint16_t value = (uint16_t)strtoul(tok.text.c_str(), NULL, 16);

        data.push_back(LOBYTE(value));
        if (value > 0xff) data.push_back(HIBYTE(value));
    }

    return true;
}

/**
 * Parse one line's tokens into a statement (pass 1).
 */
void Assembler::parseLine(const std::vector<Token>& tokens, int lineno)
{
    size_t i = 0;

    if (tokens.empty()) return;

    // Optional location: a $addr token first on the line
    if (tokens[0].kind == Token::kNumber && tokens[0].hex)
    {
        if (tokens[0].text.size() > 4)
        {
            error(lineno, tokens[0].col, "wrong number of digits in hex value, ->$%s<-",
                  tokens[0].text.c_str());
            return;
        }
        pc_ = (uint16_t)tokens[0].value;
        i++;
    }
    // Optional label: an identifier in column 1
    else if (tokens[0].kind == Token::kIdentifier && tokens[0].col == 1)
    {
        const std::string& label = tokens[0].text;

        if (labels_.find(label) != labels_.end())
        {
            error(lineno, tokens[0].col, "label %s is already defined", label.c_str());
            return;
        }

        FTRACE("Assembler recording label: %s at %04x",
            __FILE__, __LINE__, label.c_str(), pc_);

        labels_[label] = pc_;
        i++;
    }

    if (i >= tokens.size()) return;

    Statement statement;
    statement.address = pc_;
    statement.line = lineno;
    statement.opcode = 0;

    const Token& tok = tokens[i];

    if (tok.kind == Token::kDirective)
    {
        if (tok.text != ".DATA")
        {
            error(lineno, tok.col, "unknown directive %s", tok.text.c_str());
            return;
        }

        statement.kind = Statement::kData;
        if (!parseData(tokens, i + 1, lineno, statement.data)) return;
        pc_ += (uint16_t)statement.data.size();
    }
    else if (tok.kind == Token::kIdentifier)
    {
        int opcode = asmLookupInstruction(tok.text.c_str());

        if (opcode < 0)
        {
            error(lineno, tok.col, "unknown instruction %s", tok.text.c_str());
            return;
        }

        statement.kind = Statement::kInstruction;
        statement.opcode = (uint8_t)opcode;
        if (!parseOperand(tokens, i + 1, statement.opcode, lineno, statement.operand)) return;
        pc_ += asmInstructionBytes(statement.opcode);
    }
    else
    {
        error(lineno, tok.col, "expected an instruction, got ->%s%s<-",
              tok.hex ? "$" : "", tok.text.c_str());
        return;
    }

    statements_.push_back(statement);
}

/**
 * Write every statement's bytes into memory, resolving labels (pass 2).
 */
void Assembler::emit()
{
    for (size_t s = 0; s < statements_.size(); s++)
    {
        const Statement& statement = statements_[s];
        uint16_t ip = statement.address;

        if (statement.kind == Statement::kData)
        {
            for (size_t d = 0; d < statement.data.size(); d++)
            {
                memory_[ip++] = statement.data[d];
            }
            continue;
        }

        FTRACE("Assembler storing instruction: %02x at %04x",
            __FILE__, __LINE__, statement.opcode, ip);

        memory_[ip++] = statement.opcode;

        const Operand& operand = statement.operand;
        uint16_t value = operand.value;

        if (operand.kind == Operand::kNone) continue;

        if (operand.kind == Operand::kLabel)
        {
            std::map<std::string, uint16_t>::const_iterator it = labels_.find(operand.label);

            if (it == labels_.end())
            {
                error(statement.line, operand.col, "undefined label %s",
                      operand.label.c_str());
                continue;
            }

            uint16_t target = it->second;

            if (isBranch(statement.opcode))
            {
                // Relative to the address following the operand byte
                int delta = (int)target - (int)(uint16_t)(ip + 1);

                if (delta < -128 || delta > 127)
                {
                    error(statement.line, operand.col,
                          "branch to %s ($%04x) from $%04x is out of range (%d bytes)",
                          operand.label.c_str(), target, statement.address, delta);
                    continue;
                }
                value = (uint16_t)(delta & 0xff);
            }
            else if (operand.width == 1 && target > 0xff)
            {
                error(statement.line, operand.col,
                      "label %s ($%04x) does not fit in a 1-byte operand",
                      operand.label.c_str(), target);
                continue;
            }
            else
            {
                value = target;
            }
        }

        memory_[ip++] = LOBYTE(value);
        if (operand.width == 2) memory_[ip++] = HIBYTE(value);
    }
}

/**
 * Read and assemble the file.
 */
int Assembler::run()
{
    FILE* fp = fopen(filename_, "r");

    if (fp == NULL) return -1;

    char line[kMaxLineLength + 1];
    int  lineno = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        lineno++;

        FTRACE("Assembler read line: %s", __FILE__, __LINE__, line);

        std::vector<Token> tokens;
        if (lexLine(line, lineno, tokens))
        {
            parseLine(tokens, lineno);
        }
    }

    bool readError = (ferror(fp) != 0);
    fclose(fp);

    if (readError)
    {
        error(lineno, 1, "read error: %s", strerror(errno));
        return errors_;
    }

    emit();

    return errors_;
}

} // namespace

int asmAssemble(const char* filename, uint8_t* memory)
{
    Assembler assembler(filename, memory);
    return assembler.run();
}
