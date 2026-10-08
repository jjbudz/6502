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
 *    |                  an identifier (mnemonic, label or register), a
 *    |                  directive (.DATA), '#', or punctuation ( ) , :.
 *    |                  Drops comments. Remembers the column of every token
 *    |                  for error messages.
 *    |
 *    +-- parseLine()    vector<Token> -> Statement (appended to statements_)
 *    |     |            Recognises the line shape
 *    |     |                [$addr] [label[:]] MNEMONIC [operand]
 *    |     |                [$addr] [label[:]] .DATA value ...
 *    |     |            A leading $addr moves the location counter (pc_).
 *    |     |            A label is recorded in labels_ at pc_. The statement
 *    |     |            is stamped with pc_, then pc_ is advanced by the
 *    |     |            statement's size.
 *    |     |
 *    |     +-- parseOperandShape()  reads the operand's punctuation into a
 *    |     |                        Shape: #v, v, v,X, v,Y, (v,X), (v),Y, (v)
 *    |     |                        or A, with v a number or a label
 *    |     +-- encodeLegacy()       for a suffixed mnemonic (LDAZX, ...):
 *    |     |                        the mode is in the name; check the
 *    |     |                        operand is the right width
 *    |     +-- encodeStandard()     for a bare mnemonic (LDA, ...): choose
 *    |     |                        the mode from the shape and the value,
 *    |     |                        then look up (mnemonic, mode) -> opcode
 *    |     +-- parseData()          converts .DATA values to bytes
 *    |
 *    |  after the whole file has been read (pass 2):
 *    |
 *    +-- emit()         vector<Statement> -> bytes in memory_
 *                       Writes each statement at its recorded address.
 *                       Label operands are looked up here, which is why a
 *                       branch can refer to a label defined further down.
 *
 * Why two passes: an instruction's size is settled in pass 1 (by the
 * mnemonic for the legacy form; by the operand's shape and, for zero page
 * vs. absolute, by whether its value is already known for the standard
 * form), so pass 1 can assign every address. Pass 2 then has the complete
 * label table.
 *
 * Errors do not stop assembly. error() prints "file:line:col: error: ..."
 * and counts; parseLine() abandons the offending line and run() moves on,
 * so one run reports everything wrong with a file. run() returns the count.
 *
 * SYNTAX ACCEPTED
 *
 *   [$addr] [label[:]] MNEMONIC [operand]   ; comment
 *   [$addr] [label[:]] .DATA value ...      ; comment
 *
 * Two mnemonic styles are accepted and may be mixed in one file:
 *
 *   Standard   The bare 6502 mnemonic; the operand's shape selects the
 *              addressing mode, as in any 6502 assembler:
 *                LDA #$10   LDA $10     LDA $10,X    LDA $1234   LDA $1234,X
 *                LDA $1234,Y   LDA ($10,X)   LDA ($10),Y   JMP ($1234)
 *                LSR A      BNE LOOP    BNE $4010
 *              A value under $100 written with fewer than four hex digits
 *              selects zero page when the instruction has a zero page form;
 *              a label selects zero page only if it was defined earlier in
 *              the file and is under $100. Branch targets may be labels or
 *              literal addresses and must be within -128..+127.
 *
 *   Legacy     The addressing mode is spelled in the mnemonic (LDAI, LDAZ,
 *              LDAZX, LDAIX, STAA, ...) and the operand is #$hh, #ddd, $hh,
 *              $hhhh or a label. A 3-byte instruction requires a 4-digit
 *              address; a 2-byte one a value under $100 in fewer digits.
 *
 * A label is an identifier in column 1, or any identifier followed by ':'.
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
 * Addressing modes and the (mnemonic, mode) -> opcode table
 * ---------------------------------------------------------------------------
 */

/**
 * The 6502 addressing modes. kImplied also covers the accumulator mode of
 * ASL/LSR/ROL/ROR, which is one byte like the implied instructions.
 */
enum Mode
{
    kImplied,
    kImmediate,
    kZeroPage,
    kZeroPageX,
    kZeroPageY,
    kAbsolute,
    kAbsoluteX,
    kAbsoluteY,
    kIndirect,
    kIndirectX,
    kIndirectY,
    kRelative,
    kModeCount
};

const char* kModeNames[kModeCount] =
{
    "implied", "immediate", "zero page", "zero page,X", "zero page,Y",
    "absolute", "absolute,X", "absolute,Y", "(indirect)", "(indirect,X)",
    "(indirect),Y", "relative"
};

/**
 * Operand bytes for each mode.
 */
const uint8_t kModeWidth[kModeCount] = { 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1 };

/**
 * Opcodes for one bare mnemonic (e.g. "LDA"), one per mode; -1 where the
 * 6502 has no such form.
 */
struct OpcodeSet
{
    int opcode[kModeCount];
};

typedef std::map<std::string, OpcodeSet> OpcodeTable;

/**
 * Work out the addressing mode a legacy mnemonic spells. The emulator's
 * table names each opcode as a three-letter base plus a suffix:
 *
 *   (none)  implied if 1 byte, relative if 2 (the branches), absolute if 3
 *           (JMP, JSR, BIT)
 *   I       immediate if 2 bytes, (indirect) if 3 (JMPI)
 *   Z ZX ZY zero page, zero page,X, zero page,Y
 *   A X Y   absolute, absolute,X, absolute,Y
 *   IX IY   (indirect,X), (indirect),Y
 *
 * Returns kModeCount for a suffix it does not recognise.
 */
Mode modeFromSuffix(const char* suffix, uint8_t bytes)
{
    if (bytes == 1) return kImplied;

    if (strcmp(suffix, "") == 0)   return (bytes == 2) ? kRelative : kAbsolute;
    if (strcmp(suffix, "I") == 0)  return (bytes == 2) ? kImmediate : kIndirect;
    if (strcmp(suffix, "Z") == 0)  return kZeroPage;
    if (strcmp(suffix, "ZX") == 0) return kZeroPageX;
    if (strcmp(suffix, "ZY") == 0) return kZeroPageY;
    if (strcmp(suffix, "A") == 0)  return kAbsolute;
    if (strcmp(suffix, "X") == 0)  return kAbsoluteX;
    if (strcmp(suffix, "Y") == 0)  return kAbsoluteY;
    if (strcmp(suffix, "IX") == 0) return kIndirectX;
    if (strcmp(suffix, "IY") == 0) return kIndirectY;

    return kModeCount;
}

/**
 * Build the standard-syntax table from the emulator's instruction table,
 * once. Every legacy name LDAZX, LDAI, ... contributes one (LDA, mode)
 * entry, so the two syntaxes can never disagree about an opcode.
 */
const OpcodeTable& opcodeTable()
{
    static OpcodeTable table;
    static bool built = false;

    if (built) return table;
    built = true;

    for (int op = 0; op < 256; op++)
    {
        uint8_t bytes = asmInstructionBytes((uint8_t)op);
        const char* symbol = asmInstructionSymbol((uint8_t)op);

        if (bytes == 0 || strlen(symbol) < 3) continue;

        std::string base(symbol, 3);
        Mode mode = modeFromSuffix(symbol + 3, bytes);

        if (mode == kModeCount) continue;

        OpcodeTable::iterator it = table.find(base);
        if (it == table.end())
        {
            OpcodeSet empty;
            for (int m = 0; m < kModeCount; m++) empty.opcode[m] = -1;
            it = table.insert(std::make_pair(base, empty)).first;
        }

        if (it->second.opcode[mode] < 0) it->second.opcode[mode] = op;
    }

    return table;
}

/*
 * ---------------------------------------------------------------------------
 * Data passed between the stages
 * ---------------------------------------------------------------------------
 */

/**
 * One word of a source line, as produced by lexLine().
 *
 * The lexer only classifies; it does not know whether an identifier is a
 * mnemonic, a label or the register X, or whether a number is a sensible
 * size. Those are the parser's decisions, which is why text, hex and value
 * are all kept.
 */
struct Token
{
    enum Kind
    {
        kNumber,     // $hex or decimal digits
        kIdentifier, // mnemonic, label, or A/X/Y
        kDirective,  // .DATA
        kHash,       // '#', introduces an immediate value
        kPunct       // one of ( ) , :
    };

    Kind          kind;
    std::string   text;   // source text, uppercased, without any '$' prefix
    bool          hex;    // kNumber: had a '$' prefix
    unsigned long value;  // kNumber: numeric value
    int           col;    // 1-based column of the first character
};

/**
 * The punctuation pattern of an operand, as read by parseOperandShape(),
 * before any decision about addressing mode. 'v' is a number or label.
 */
struct Shape
{
    enum Kind
    {
        kNone,         //
        kAccumulator,  // A
        kImmediate,    // #v
        kDirect,       // v
        kDirectX,      // v,X
        kDirectY,      // v,Y
        kIndirect,     // (v)
        kIndirectX,    // (v,X)
        kIndirectY     // (v),Y
    };

    Kind  kind;
    Token value;  // the v token; meaningful unless kNone/kAccumulator

    Shape() : kind(kNone) {}
};

/**
 * An instruction's encoded operand, as produced by encodeLegacy() or
 * encodeStandard(). width and relative are final; the value is final for
 * kLiteral, and for kLabel is looked up by emit().
 */
struct Operand
{
    enum Kind
    {
        kNone,
        kLiteral, // value is known
        kLabel    // value is labels_[label], resolved in pass 2
    };

    Kind        kind;
    uint16_t    value;
    std::string label;
    uint8_t     width;    // operand bytes: 0, 1 or 2
    bool        relative; // encode as a branch offset from the next instruction
    int         col;      // for error messages in pass 2

    Operand() : kind(kNone), value(0), width(0), relative(false), col(0) {}
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
 * are the opcodes xxx10000; they are the only ones whose operand is encoded
 * as an offset rather than an address.
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
    bool parseOperandShape(const std::vector<Token>& tokens, size_t first,
                           int lineno, Shape& shape);
    bool encodeLegacy(const Shape& shape, uint8_t opcode, int lineno,
                      Operand& operand);
    bool encodeStandard(const Shape& shape, const std::string& mnemonic,
                        const OpcodeSet& set, int lineno, int col,
                        uint8_t& opcode, Operand& operand);
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
 *   ( ) , :          kPunct (one-character tokens, so "($10),Y" lexes
 *                    without spaces)
 *   '$' or digit     kNumber; the digits that follow are validated against
 *                    the radix here so later stages can trust token.value
 *   '.'              kDirective
 *   letter or '_'    kIdentifier
 *   anything else    error
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
        else if (c == '(' || c == ')' || c == ',' || c == ':')
        {
            token.kind = Token::kPunct;
            token.text = std::string(1, c);
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
 * Read the operand tokens (tokens[first..]) into a Shape: which of the
 * 6502 operand patterns they form, and the value token inside it.
 *
 *   (nothing)          kNone
 *   A                  kAccumulator
 *   # v                kImmediate
 *   v                  kDirect
 *   v , X   /  v , Y   kDirectX / kDirectY
 *   ( v )              kIndirect
 *   ( v , X )          kIndirectX
 *   ( v ) , Y          kIndirectY
 *
 * where v is a number or an identifier. This is pure syntax: whether the
 * instruction supports the pattern is decided by encodeLegacy() or
 * encodeStandard(). Returns false after reporting an error.
 */
bool Assembler::parseOperandShape(const std::vector<Token>& tokens, size_t first,
                                  int lineno, Shape& shape)
{
    size_t i = first;
    size_t n = tokens.size();

    if (i >= n)
    {
        shape.kind = Shape::kNone;
        return true;
    }

    // Helpers expressed as small lambdas would be neater, but this file is
    // kept to the C++ the rest of the project uses.
    #define IS_PUNCT(idx, ch) ((idx) < n && tokens[idx].kind == Token::kPunct && tokens[idx].text[0] == (ch))
    #define IS_REG(idx, name) ((idx) < n && tokens[idx].kind == Token::kIdentifier && tokens[idx].text == (name))
    #define IS_VALUE(idx)     ((idx) < n && (tokens[idx].kind == Token::kNumber || tokens[idx].kind == Token::kIdentifier))

    if (IS_REG(i, "A") && i + 1 == n)
    {
        shape.kind = Shape::kAccumulator;
        shape.value = tokens[i];
        return true;
    }

    if (tokens[i].kind == Token::kHash) // # v
    {
        if (!IS_VALUE(i + 1))
        {
            error(lineno, tokens[i].col, "expected a value after #");
            return false;
        }
        shape.kind = Shape::kImmediate;
        shape.value = tokens[i + 1];
        i += 2;
    }
    else if (IS_PUNCT(i, '(')) // ( v ...
    {
        if (!IS_VALUE(i + 1))
        {
            error(lineno, tokens[i].col, "expected a value after (");
            return false;
        }
        shape.value = tokens[i + 1];

        if (IS_PUNCT(i + 2, ',')) // ( v , X )
        {
            if (!IS_REG(i + 3, "X") || !IS_PUNCT(i + 4, ')'))
            {
                error(lineno, tokens[i].col, "expected (value,X)");
                return false;
            }
            shape.kind = Shape::kIndirectX;
            i += 5;
        }
        else if (IS_PUNCT(i + 2, ')'))
        {
            if (IS_PUNCT(i + 3, ',')) // ( v ) , Y
            {
                if (!IS_REG(i + 4, "Y"))
                {
                    error(lineno, tokens[i + 3].col, "expected (value),Y");
                    return false;
                }
                shape.kind = Shape::kIndirectY;
                i += 5;
            }
            else // ( v )
            {
                shape.kind = Shape::kIndirect;
                i += 3;
            }
        }
        else
        {
            error(lineno, tokens[i].col, "unbalanced parenthesis in operand");
            return false;
        }
    }
    else if (IS_VALUE(i)) // v [, X|Y]
    {
        shape.value = tokens[i];
        shape.kind = Shape::kDirect;
        i++;

        if (IS_PUNCT(i, ','))
        {
            if (IS_REG(i + 1, "X"))      shape.kind = Shape::kDirectX;
            else if (IS_REG(i + 1, "Y")) shape.kind = Shape::kDirectY;
            else
            {
                error(lineno, tokens[i].col, "expected ,X or ,Y");
                return false;
            }
            i += 2;
        }
    }
    else
    {
        error(lineno, tokens[i].col, "unexpected token ->%s<-", tokens[i].text.c_str());
        return false;
    }

    #undef IS_PUNCT
    #undef IS_REG
    #undef IS_VALUE

    // Anything left over is a second operand, which no 6502 instruction has
    if (i < n)
    {
        error(lineno, tokens[i].col, "unexpected token after operand ->%s<-",
              tokens[i].text.c_str());
        return false;
    }

    return true;
}

/**
 * Encode the operand for a legacy mnemonic, whose addressing mode is spelled
 * in its name (LDAZX is always zero page,X). All that remains is to check
 * the operand written in the source is the width the instruction needs, so
 * the emitted code can never be misaligned:
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
 * kept so existing sources assemble to identical bytes. The standard forms
 * v,X  (v)  and so on are not accepted here: the suffix already said that.
 *
 * Returns false after reporting an error.
 */
bool Assembler::encodeLegacy(const Shape& shape, uint8_t opcode, int lineno,
                             Operand& operand)
{
    uint8_t bytes = asmInstructionBytes(opcode);
    const Token& tok = shape.value;

    if (shape.kind == Shape::kNone)
    {
        if (bytes > 1)
        {
            error(lineno, 0, "missing operand");
            return false;
        }
        return true;
    }

    if (bytes == 1)
    {
        error(lineno, tok.col, "instruction takes no operand");
        return false;
    }

    operand.col = tok.col;
    operand.width = bytes - 1;
    operand.relative = isBranch(opcode);

    if (shape.kind == Shape::kImmediate)
    {
        if (tok.kind != Token::kNumber)
        {
            error(lineno, tok.col, "expected a value after #");
            return false;
        }
        if (tok.hex && tok.text.size() > 2)
        {
            error(lineno, tok.col, "wrong number of digits in hex value, ->$%s<-",
                  tok.text.c_str());
            return false;
        }
        if (!tok.hex && tok.text.size() > 3)
        {
            error(lineno, tok.col, "wrong number of digits in decimal value, ->%s<-",
                  tok.text.c_str());
            return false;
        }
        if (tok.value > 0xff)
        {
            error(lineno, tok.col, "immediate value out of range, ->%s<-",
                  tok.text.c_str());
            return false;
        }

        operand.kind = Operand::kLiteral;
        operand.value = (uint16_t)tok.value;
        operand.width = 1;
        return true;
    }

    if (shape.kind != Shape::kDirect)
    {
        error(lineno, tok.col,
              "%s spells its addressing mode; use the bare mnemonic for standard syntax",
              asmInstructionSymbol(opcode));
        return false;
    }

    if (tok.kind == Token::kIdentifier) // label, resolved in pass 2
    {
        operand.kind = Operand::kLabel;
        operand.label = tok.text;
        return true;
    }

    // Address literal
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

    // Four digits always mean a two-byte address, even $0040; fewer digits
    // mean one byte as long as the value fits.
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

    operand.kind = Operand::kLiteral;
    operand.value = (uint16_t)tok.value;
    return true;
}

/**
 * Encode the operand for a bare mnemonic by choosing the addressing mode
 * from the operand's shape and value, then looking up (mnemonic, mode).
 *
 *   shape           candidate modes
 *   (none) / A      implied
 *   #v              immediate
 *   v               relative if the mnemonic is a branch; else zero page
 *                   or absolute
 *   v,X   v,Y       zero page,X or absolute,X;  zero page,Y or absolute,Y
 *   (v)             (indirect)
 *   (v,X)  (v),Y    (indirect,X)  (indirect),Y
 *
 * Zero page is chosen over absolute when the mnemonic has a zero page form
 * and the value is known to fit: a literal under $100 written with fewer
 * than four digits, or a label already defined at an address under $100.
 * A label defined later in the file is assumed absolute, so that pass 1
 * can fix the instruction's size without knowing the label. When only the
 * zero page form exists (STX v,Y; every (indirect,X) and (indirect),Y) the
 * value must fit in a byte, which emit() checks for labels.
 *
 * Returns false after reporting an error.
 */
bool Assembler::encodeStandard(const Shape& shape, const std::string& mnemonic,
                               const OpcodeSet& set, int lineno, int col,
                               uint8_t& opcode, Operand& operand)
{
    const Token& tok = shape.value;

    // Is the value known now, and does it fit in zero page?
    unsigned long value = 0;
    bool fitsZeroPage = false;

    if (shape.kind != Shape::kNone && shape.kind != Shape::kAccumulator)
    {
        if (tok.kind == Token::kNumber)
        {
            if (tok.hex && tok.text.size() > 4)
            {
                error(lineno, tok.col, "wrong number of digits in hex value, ->$%s<-",
                      tok.text.c_str());
                return false;
            }
            if (tok.value > 0xffff)
            {
                error(lineno, tok.col, "value out of range, ->%s<-", tok.text.c_str());
                return false;
            }
            value = tok.value;
            fitsZeroPage = (value <= 0xff) && !(tok.hex && tok.text.size() == 4);
        }
        else
        {
            std::map<std::string, uint16_t>::const_iterator it = labels_.find(tok.text);
            if (it != labels_.end())
            {
                value = it->second;
                fitsZeroPage = (value <= 0xff);
            }
        }
    }

    // Pick the mode from the shape
    Mode mode = kModeCount;

    switch (shape.kind)
    {
    case Shape::kNone:
    case Shape::kAccumulator:
        mode = kImplied;
        break;

    case Shape::kImmediate:
        mode = kImmediate;
        break;

    case Shape::kIndirect:
        mode = kIndirect;
        break;

    case Shape::kIndirectX:
        mode = kIndirectX;
        break;

    case Shape::kIndirectY:
        mode = kIndirectY;
        break;

    case Shape::kDirect:
    case Shape::kDirectX:
    case Shape::kDirectY:
        {
            Mode zp  = (shape.kind == Shape::kDirect) ? kZeroPage :
                       (shape.kind == Shape::kDirectX) ? kZeroPageX : kZeroPageY;
            Mode abs = (shape.kind == Shape::kDirect) ? kAbsolute :
                       (shape.kind == Shape::kDirectX) ? kAbsoluteX : kAbsoluteY;

            if (shape.kind == Shape::kDirect && set.opcode[kRelative] >= 0)
            {
                mode = kRelative;
            }
            else if (fitsZeroPage && set.opcode[zp] >= 0)
            {
                mode = zp;
            }
            else if (set.opcode[abs] >= 0)
            {
                mode = abs;
            }
            else
            {
                mode = zp; // only form there is; emit() checks the value fits
            }
        }
        break;
    }

    if (mode == kModeCount || set.opcode[mode] < 0)
    {
        error(lineno, tok.col ? tok.col : col, "%s has no %s form",
              mnemonic.c_str(), kModeNames[mode == kModeCount ? kImplied : mode]);
        return false;
    }

    opcode = (uint8_t)set.opcode[mode];
    operand.width = kModeWidth[mode];
    operand.relative = (mode == kRelative);
    operand.col = tok.col;

    if (operand.width == 0) return true;

    if (tok.kind == Token::kIdentifier)
    {
        // A label is always resolved in pass 2 (even if known now) so that
        // one code path checks ranges and computes branch offsets.
        operand.kind = Operand::kLabel;
        operand.label = tok.text;
        return true;
    }

    if (operand.width == 1 && !operand.relative && value > 0xff)
    {
        error(lineno, tok.col, "%s %s takes a 1-byte value, got ->%s%s<-",
              mnemonic.c_str(), kModeNames[mode], tok.hex ? "$" : "", tok.text.c_str());
        return false;
    }

    operand.kind = Operand::kLiteral;
    operand.value = (uint16_t)value;
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
 *     [$addr] [label[:]] MNEMONIC [operand]
 *     [$addr] [label[:]] .DATA value ...
 *
 *   1. A leading $addr sets the location counter.
 *   2. An identifier in column 1, or any identifier followed by ':',
 *      defines a label at the current location. Column 1 is the rule for
 *      the colon-less form so an indented mnemonic is never mistaken for a
 *      label and a label is never confused with an operand elsewhere on
 *      the line.
 *   3. What remains is a directive or a mnemonic. The operand's shape is
 *      read first (parseOperandShape), then:
 *        - a name in the emulator's table with a mode suffix (LDAZX) is a
 *          legacy mnemonic: encodeLegacy()
 *        - a three-letter name in the standard table (LDA): encodeStandard()
 *        - anything else is an error rather than a guess.
 *
 * The statement is stamped with the current pc_, then pc_ advances by the
 * statement's size, which is why every later label gets the right address
 * without emitting anything yet. On error the line contributes nothing.
 */
void Assembler::parseLine(const std::vector<Token>& tokens, int lineno)
{
    size_t i = 0;
    size_t n = tokens.size();

    if (n == 0) return; // blank or comment-only line

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

    // 2. Optional label: an identifier in column 1, or one followed by ':'
    if (i < n && tokens[i].kind == Token::kIdentifier)
    {
        bool colon = (i + 1 < n && tokens[i+1].kind == Token::kPunct &&
                      tokens[i+1].text == ":");

        if (colon || tokens[i].col == 1)
        {
            const std::string& label = tokens[i].text;

            if (labels_.find(label) != labels_.end())
            {
                error(lineno, tokens[i].col, "label %s is already defined", label.c_str());
                return;
            }

            FTRACE("Assembler recording label: %s at %04x",
                __FILE__, __LINE__, label.c_str(), pc_);

            labels_[label] = pc_;
            i += colon ? 2 : 1;
        }
    }

    if (i >= n) return; // just a location or a label on its own

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
        Shape shape;
        if (!parseOperandShape(tokens, i + 1, lineno, shape)) return;

        statement.kind = Statement::kInstruction;

        int legacy = asmLookupInstruction(tok.text.c_str());
        const OpcodeTable& table = opcodeTable();
        OpcodeTable::const_iterator standard = table.find(tok.text);

        if (legacy >= 0 && tok.text.size() > 3) // suffixed legacy mnemonic
        {
            statement.opcode = (uint8_t)legacy;
            if (!encodeLegacy(shape, statement.opcode, lineno, statement.operand)) return;
        }
        else if (standard != table.end()) // bare mnemonic
        {
            if (!encodeStandard(shape, tok.text, standard->second, lineno, tok.col,
                                statement.opcode, statement.operand)) return;
        }
        else
        {
            error(lineno, tok.col, "unknown instruction %s", tok.text.c_str());
            return;
        }

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
 * number. Then, for a label or a literal alike:
 *
 *   relative (branches)      one byte: the signed distance from the address
 *                            after the operand byte to the target, which
 *                            must be within -128..127
 *   one-byte operand         the value must fit in a byte (zero page, or
 *                            an immediate given as a label)
 *   two-byte operand         low byte first
 *
 * An unresolvable or out-of-range operand is reported at the line and
 * column of the operand and left unwritten.
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

            value = it->second;
        }

        if (operand.relative)
        {
            // The CPU adds the offset to the PC after it has fetched the
            // whole two-byte instruction, i.e. to ip + 1 here.
            int delta = (int)value - (int)(uint16_t)(ip + 1);

            if (delta < -128 || delta > 127)
            {
                error(statement.line, operand.col,
                      "branch to $%04x from $%04x is out of range (%d bytes)",
                      value, statement.address, delta);
                continue;
            }
            value = (uint16_t)(delta & 0xff);
        }
        else if (operand.width == 1 && value > 0xff)
        {
            error(statement.line, operand.col,
                  "value $%04x does not fit in a 1-byte operand", value);
            continue;
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
