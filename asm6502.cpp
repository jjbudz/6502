/**
 * Two-pass assembler for the 6502 emulator.
 *
 * HOW TO READ THIS FILE
 *
 * Start at asmAssemble() at the bottom; it constructs an Assembler and calls
 * Assembler::run(). run() drives everything:
 *
 *   run()
 *    |
 *    |  for each line of the source file (pass 1):
 *    |
 *    +-- lexLine()      text  -> vector<Token>
 *    |                  Splits the line into words and classifies each one
 *    |                  by its first character: a number ($hex or decimal),
 *    |                  an identifier (mnemonic or label), a directive
 *    |                  (.DATA) or '#'. Drops comments. Remembers the
 *    |                  column of every token for error messages.
 *    |
 *    +-- parseLine()    vector<Token> -> Statement (appended to statements_)
 *    |     |            Recognises the line shape
 *    |     |                [$addr] [label] MNEMONIC [operand]
 *    |     |                [$addr] [label] .DATA value ...
 *    |     |            A leading $addr moves the location counter (pc_).
 *    |     |            A label in column 1 is recorded in labels_ at pc_.
 *    |     |            The statement is stamped with pc_, then pc_ is
 *    |     |            advanced by the statement's size.
 *    |     |
 *    |     +-- parseOperand()  checks the operand matches the instruction
 *    |     |                   (immediate / address / label) and the width
 *    |     |                   the instruction needs
 *    |     +-- parseData()     converts .DATA values to bytes
 *    |
 *    |  after the whole file has been read (pass 2):
 *    |
 *    +-- emit()         vector<Statement> -> bytes in memory_
 *                       Writes each statement at its recorded address.
 *                       Label operands are looked up here, which is why a
 *                       branch can refer to a label defined further down.
 *
 * Why two passes: an instruction's size never depends on a label's value
 * (the mnemonic fixes it), so pass 1 can assign every address without
 * knowing any label. Pass 2 then has the complete label table.
 *
 * Errors do not stop assembly. error() prints "file:line:col: error: ..."
 * and counts; parseLine() abandons the offending line and run() moves on,
 * so one run reports everything wrong with a file. run() returns the count.
 *
 * SYNTAX ACCEPTED (legacy form; see docs/ASSEMBLER_PLAN.md for what follows)
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

/*
 * ---------------------------------------------------------------------------
 * Data passed between the stages
 * ---------------------------------------------------------------------------
 */

/**
 * One word of a source line, as produced by lexLine().
 *
 * The lexer only classifies; it does not know whether an identifier is a
 * mnemonic or a label, or whether a number is a sensible size. Those are
 * the parser's decisions, which is why text, hex and value are all kept.
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
 * An instruction's operand, as produced by parseOperand().
 *
 * For kImmediate and kAddress the value is final. For kLabel only the name
 * is known in pass 1; emit() looks it up and decides how to encode it
 * (relative offset for a branch, address otherwise).
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
    int         col;   // for error messages in pass 2

    Operand() : kind(kNone), value(0), width(0), col(0) {}
};

/**
 * One assembled item, as produced by parseLine(): an instruction with its
 * operand, or a run of data bytes. address is where emit() will write it.
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
    int                  line;    // for error messages in pass 2
    uint8_t              opcode;  // kInstruction
    Operand              operand; // kInstruction
    std::vector<uint8_t> data;    // kData
};

/**
 * The eight relative branch instructions (BPL BMI BVC BVS BCC BCS BNE BEQ)
 * are the opcodes xxx10000; they are the only ones whose label operand is
 * encoded as an offset rather than an address.
 */
bool isBranch(uint8_t opcode)
{
    return (opcode & 0x1f) == 0x10;
}

/*
 * ---------------------------------------------------------------------------
 * The assembler
 * ---------------------------------------------------------------------------
 */

class Assembler
{
public:
    Assembler(const char* filename, uint8_t* memory);

    /**
     * Assemble the whole file into memory. Returns the error count, or -1
     * if the file could not be opened.
     */
    int run();

private:
    // Pass 1: text -> tokens -> statements, addresses and labels
    bool lexLine(const char* line, int lineno, std::vector<Token>& tokens);
    void parseLine(const std::vector<Token>& tokens, int lineno);
    bool parseOperand(const std::vector<Token>& tokens, size_t first,
                      uint8_t opcode, int lineno, Operand& operand);
    bool parseData(const std::vector<Token>& tokens, size_t first,
                   int lineno, std::vector<uint8_t>& data);

    // Pass 2: statements -> bytes, labels resolved
    void emit();

    void error(int line, int col, const char* fmt, ...);

    const char*                     filename_;   // for error messages
    uint8_t*                        memory_;     // the emulator's 64K image
    uint16_t                        pc_;         // location counter during pass 1
    int                             errors_;
    std::map<std::string, uint16_t> labels_;     // name -> address, filled in pass 1
    std::vector<Statement>          statements_; // in source order, filled in pass 1
};

Assembler::Assembler(const char* filename, uint8_t* memory)
    : filename_(filename), memory_(memory), pc_(0), errors_(0)
{
    assert(filename);
    assert(memory);
}

/**
 * Report an error at a source position and count it. Nothing is thrown or
 * exited; the caller decides how much of the current line to abandon, and
 * run() reports the total.
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

/*
 * ---------------------------------------------------------------------------
 * Pass 1
 * ---------------------------------------------------------------------------
 */

/**
 * Lexer: split one line into tokens.
 *
 * Walks the line character by character. Whitespace separates tokens; ';'
 * ends the line. The first character of a word decides its kind:
 *
 *   '#'              kHash (a one-character token)
 *   '$' or digit     kNumber; the digits that follow are validated against
 *                    the radix here so later stages can trust token.value
 *   '.'              kDirective
 *   letter or '_'    kIdentifier
 *   anything else    error (for example ',' or '(' from standard 6502
 *                    syntax, which this version does not accept)
 *
 * Text is uppercased so the rest of the assembler is case-insensitive.
 * Returns false after reporting an error; the caller skips the line.
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

        if (c == ';') break; // comment runs to end of line

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
            if (token.hex) i++; // the '$' itself is not part of the text

            // Collect every alphanumeric so that a malformed number such
            // as $12G4 is reported as one bad value rather than split in two.
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

            // The parser checks digit counts (2 for a byte, 4 for an
            // address) and reports them; here just avoid overflowing on an
            // absurdly long literal.
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
 * Parse the operand tokens (tokens[first..]) for an instruction.
 *
 * The instruction's total length (asmInstructionBytes) says how many
 * operand bytes it needs: 0, 1 or 2. This function checks that the operand
 * written in the source is of that size, so the emitted code can never be
 * misaligned by an operand that is too short or too long:
 *
 *   #$hh / #ddd   one byte, must be <= 255
 *   $hh           one byte: for 2-byte instructions, value <= $FF and
 *                 written with fewer than four digits
 *   $hhhh         two bytes: for 3-byte instructions, exactly four digits
 *                 (so $0040 is an absolute address and $40 is zero page)
 *   label         width is whatever the instruction needs; the value is
 *                 filled in by emit()
 *
 * These are the rules the original assembler applied to its test programs,
 * kept so existing sources assemble to identical bytes.
 *
 * Returns false after reporting an error.
 */
bool Assembler::parseOperand(const std::vector<Token>& tokens, size_t first,
                             uint8_t opcode, int lineno, Operand& operand)
{
    uint8_t bytes = asmInstructionBytes(opcode);
    size_t  count = tokens.size() - first;

    // No operand written
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

    // Operand written for an implied-mode instruction
    if (bytes == 1)
    {
        error(lineno, tokens[first].col, "instruction takes no operand");
        return false;
    }

    const Token& tok = tokens[first];
    operand.col = tok.col;

    if (tok.kind == Token::kHash) // immediate value: '#' then a number
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
    else if (tok.kind == Token::kNumber) // address literal
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

        // Four digits always mean a two-byte address, even $0040; fewer
        // digits mean one byte as long as the value fits.
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

    // Anything left over is a second operand, which no 6502 instruction has
    if (count > 0)
    {
        error(lineno, tokens[first].col, "unexpected token after operand ->%s<-",
              tokens[first].text.c_str());
        return false;
    }

    return true;
}

/**
 * Parse the values of a .DATA directive (tokens[first..]) into bytes.
 *
 * Each value is 1-4 hex digits with an optional '$' prefix. A value above
 * $FF produces two bytes, low byte first; otherwise one byte. So
 * ".DATA $06 $1234" produces 06 34 12.
 *
 * Returns false after reporting an error.
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

        // The lexer does not know it is inside .DATA, so bare hex arrives
        // in two forms: "55" as a decimal kNumber and "AB" as a kIdentifier.
        // Both are accepted here by re-reading the text as hex digits.
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
 * Parser: turn one line's tokens into a Statement (pass 1).
 *
 * Works left to right through the line shape
 *
 *     [$addr] [label] MNEMONIC [operand]
 *     [$addr] [label] .DATA value ...
 *
 *   1. A leading $addr sets the location counter. (A label and a $addr
 *      cannot both appear: the label must be in column 1, and a $addr
 *      first on the line occupies it.)
 *   2. An identifier in column 1 defines a label at the current location.
 *      Column 1 is the rule, not "first word", so an indented mnemonic is
 *      never mistaken for a label and a label is never confused with an
 *      operand elsewhere on the line.
 *   3. What remains is a directive or a mnemonic. The mnemonic is looked
 *      up in the emulator's instruction table (asmLookupInstruction); an
 *      unknown one is an error rather than a guess.
 *
 * The statement is stamped with the current pc_, then pc_ advances by the
 * statement's size, which is why every later label gets the right address
 * without emitting anything yet. On error the line contributes nothing.
 */
void Assembler::parseLine(const std::vector<Token>& tokens, int lineno)
{
    size_t i = 0;

    if (tokens.empty()) return; // blank or comment-only line

    // 1. Optional location: a $addr token first on the line
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
    // 2. Optional label: an identifier in column 1
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

    if (i >= tokens.size()) return; // just a location or a label on its own

    // 3. The statement itself
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

/*
 * ---------------------------------------------------------------------------
 * Pass 2
 * ---------------------------------------------------------------------------
 */

/**
 * Emitter: write every statement's bytes into memory (pass 2).
 *
 * By now labels_ is complete, so this is where a label operand becomes a
 * number:
 *
 *   branch (BNE, BEQ, ...)   one byte: the signed distance from the address
 *                            after the operand byte to the label, which
 *                            must be within -128..127
 *   2-byte instruction       one byte: the label's address, which must be
 *                            in zero page
 *   3-byte instruction       two bytes: the label's address, low byte first
 *
 * Immediate and literal-address operands were finished in pass 1 and are
 * written as they are. An unresolvable label is reported at the line and
 * column of the operand and that statement's operand is left unwritten.
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
                // The CPU adds the offset to the PC after it has fetched
                // the whole two-byte instruction, i.e. to ip + 1 here.
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

/*
 * ---------------------------------------------------------------------------
 * Driver
 * ---------------------------------------------------------------------------
 */

/**
 * Read the file line by line through pass 1, then run pass 2.
 *
 * A line that fails to lex or parse is dropped and the next line is read,
 * so all of a file's errors are reported together. Pass 2 still runs after
 * pass 1 errors (it may find undefined labels too), but the caller should
 * treat any nonzero return as "do not run this program".
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

/**
 * Entry point; see asm6502.h. assemble() in l6502.cpp calls this after
 * clearing the emulator's memory.
 */
int asmAssemble(const char* filename, uint8_t* memory)
{
    Assembler assembler(filename, memory);
    return assembler.run();
}
