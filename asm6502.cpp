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
 *    |                  by its first character: a number ($hex, decimal,
 *    |                  %binary or 'c'), an identifier (mnemonic, label or
 *    |                  register), a directive (.ORG ...), a "string", '#',
 *    |                  or punctuation ( ) , : = + - < > *. Drops comments.
 *    |                  Remembers the column of every token for messages.
 *    |
 *    +-- parseLine()    vector<Token> -> Statement (appended to statements_)
 *    |     |            Recognises the line shape
 *    |     |                [$addr] [label[:]] MNEMONIC [operand]
 *    |     |                [$addr] [label[:]] .DIRECTIVE arguments
 *    |     |                NAME = expression      (or  * = expression)
 *    |     |            A leading $addr moves the location counter (pc_).
 *    |     |            A label is recorded in labels_ at pc_. The statement
 *    |     |            is stamped with pc_, then pc_ is advanced by the
 *    |     |            statement's size.
 *    |     |
 *    |     +-- parseOperandShape()  reads the operand's punctuation into a
 *    |     |                        Shape: #e, e, e,X, e,Y, (e,X), (e),Y, (e)
 *    |     |                        or A, with e an expression
 *    |     +-- encodeLegacy()       for a suffixed mnemonic (LDAZX, ...):
 *    |     |                        the mode is in the name; check the
 *    |     |                        operand is the right width
 *    |     +-- encodeStandard()     for a bare mnemonic (LDA, ...): choose
 *    |     |                        the mode from the shape and the value,
 *    |     |                        then look up (mnemonic, mode) -> opcode
 *    |     +-- parseDirective()     .ORG .BYTE .WORD .TEXT .DATA
 *    |     +-- evalExpr()           used by all of the above to compute an
 *    |                              expression's value, or to find out that
 *    |                              it depends on a label not yet defined
 *    |
 *    |  after the whole file has been read (pass 2):
 *    |
 *    +-- emit()         vector<Statement> -> bytes in memory_
 *    |                  Writes each statement at its recorded address.
 *    |                  Expressions are evaluated again here with the full
 *    |                  label table (evalExpr), which is why an operand can
 *    |                  refer to a label defined further down.
 *    |
 *    +-- writeListing() optional: address, bytes and text of every line
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
 *   [$addr] [label[:]] .DIRECTIVE args      ; comment
 *   NAME = expression                       ; constant (or NAME .EQU expression)
 *   *    = expression                       ; same as .ORG
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
 * Expressions:  value  (+|-) value ...  with unary  <  (low byte),
 *               >  (high byte) and  -  (negate). A value is a number
 *               ($1F, 31, %00011111, 'A'), a label or constant, or  *  (the
 *               address of the current instruction). Arithmetic is 16-bit.
 *
 * Directives:   .ORG e           set the location counter (e known now)
 *               .BYTE e, e, ...  one byte each; a "string" gives one byte
 *                                per character
 *               .WORD e, e, ...  two bytes each, low byte first
 *               .TEXT "s", e...  the same as .BYTE, for readability
 *               .DATA h h ...    legacy: hex values with or without '$', one
 *                                byte each or two (low first) when above $FF
 *
 * A label is an identifier in column 1, or any identifier followed by ':'.
 * Source is case-insensitive except inside strings and character literals.
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
 * size. Those are the parser's decisions, which is why text, radix and
 * value are all kept.
 */
struct Token
{
    enum Kind
    {
        kNumber,     // $hex, decimal, %binary or 'c'
        kIdentifier, // mnemonic, label, constant, or A/X/Y
        kDirective,  // .ORG .BYTE .WORD .TEXT .DATA
        kString,     // "text", kept in its original case
        kHash,       // '#', introduces an immediate value
        kPunct       // one of ( ) , : = + - < > *
    };

    enum Radix
    {
        kDecimal,
        kHex,
        kBinary,
        kChar
    };

    Kind          kind;
    std::string   text;   // source text, uppercased, without any $ % or quotes
    Radix         radix;  // kNumber
    unsigned long value;  // kNumber: numeric value
    int           col;    // 1-based column of the first character

    Token() : kind(kPunct), radix(kDecimal), value(0), col(0) {}

    bool isPunct(char c) const { return kind == kPunct && text[0] == c; }
    bool isHex() const         { return kind == kNumber && radix == kHex; }
};

typedef std::vector<Token> Expr; // an operand expression, as tokens

/**
 * What evalExpr() learns about an expression.
 */
struct ExprResult
{
    uint16_t value;     // valid when known
    bool     known;     // every label in it is defined
    bool     byteOp;    // the whole expression is < e or > e, so fits a byte
    bool     wideHex;   // a lone 4-digit hex literal, e.g. $0080: never zero page

    ExprResult() : value(0), known(false), byteOp(false), wideHex(false) {}
};

/**
 * The punctuation pattern of an operand, as read by parseOperandShape(),
 * before any decision about addressing mode. 'e' is an expression.
 */
struct Shape
{
    enum Kind
    {
        kNone,         //
        kAccumulator,  // A
        kImmediate,    // #e
        kDirect,       // e
        kDirectX,      // e,X
        kDirectY,      // e,Y
        kIndirect,     // (e)
        kIndirectX,    // (e,X)
        kIndirectY     // (e),Y
    };

    Kind kind;
    Expr expr;  // the e tokens; empty for kNone/kAccumulator
    int  col;   // column of the first operand token

    Shape() : kind(kNone), col(0) {}
};

/**
 * An instruction's encoded operand, as produced by encodeLegacy() or
 * encodeStandard(). width and relative are final; the value comes from
 * evaluating expr in pass 2, when every label is known.
 */
struct Operand
{
    Expr     expr;      // empty for an instruction with no operand
    uint16_t pc;        // address of the instruction, for '*' in expr
    uint8_t  width;     // operand bytes: 0, 1 or 2
    bool     relative;  // encode as a branch offset from the next instruction
    bool     immediate; // may hold a negative byte (-128..-1 as $80..$FF)
    int      col;       // for error messages in pass 2

    Operand() : pc(0), width(0), relative(false), immediate(false), col(0) {}
};

/**
 * One value of a data directive: an expression and the bytes it occupies.
 */
struct DataItem
{
    Expr    expr;
    uint8_t width; // 1 or 2
    int     col;
};

/**
 * One assembled item, as produced by parseLine(): an instruction with its
 * operand, or a run of data. address is where emit() will write it.
 */
struct Statement
{
    enum Kind
    {
        kInstruction,
        kData
    };

    Kind                  kind;
    uint16_t              address;
    int                   line;    // for error messages in pass 2
    uint8_t               opcode;  // kInstruction
    Operand               operand; // kInstruction
    std::vector<DataItem> data;    // kData
};

/**
 * What the listing shows for one source line: the address the line
 * occupies or defines, the bytes it generated, or a constant's value.
 */
struct LineInfo
{
    bool                 hasAddress;
    uint16_t             address;
    bool                 isConstant;
    uint16_t             value;    // isConstant
    std::vector<uint8_t> bytes;

    LineInfo() : hasAddress(false), address(0), isConstant(false), value(0) {}

    void setAddress(uint16_t a) { if (!hasAddress) { hasAddress = true; address = a; } }
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

/**
 * A single-token expression holding a known value, for data the parser
 * has already turned into bytes (.DATA values, .TEXT characters).
 */
Expr literalExpr(unsigned long value, int col)
{
    Token tok;
    tok.kind = Token::kNumber;
    tok.radix = Token::kDecimal;
    tok.value = value;
    tok.col = col;

    char text[16];
    snprintf(text, sizeof(text), "%lu", value);
    tok.text = text;

    return Expr(1, tok);
}

/*
 * ---------------------------------------------------------------------------
 * The assembler
 * ---------------------------------------------------------------------------
 */

class Assembler
{
public:
    Assembler(const char* filename, uint8_t* memory, FILE* listing);

    /**
     * Assemble the whole file into memory. Returns the error count, or -1
     * if the file could not be opened.
     */
    int run();

private:
    // Pass 1: text -> tokens -> statements, addresses and labels
    bool lexLine(const char* line, int lineno, std::vector<Token>& tokens);
    void parseLine(const std::vector<Token>& tokens, int lineno);
    bool defineLabel(const Token& name, uint16_t value, int lineno);
    bool parseOperandShape(const std::vector<Token>& tokens, size_t first,
                           int lineno, Shape& shape);
    bool encodeLegacy(const Shape& shape, uint8_t opcode, int lineno,
                      Operand& operand);
    bool encodeStandard(const Shape& shape, const std::string& mnemonic,
                        const OpcodeSet& set, int lineno, int col,
                        uint8_t& opcode, Operand& operand);
    bool parseDirective(const std::vector<Token>& tokens, size_t first,
                        int lineno, Statement& statement, bool& isStatement);
    bool parseDataLegacy(const std::vector<Token>& tokens, size_t first,
                         int lineno, std::vector<DataItem>& data);
    bool splitArguments(const std::vector<Token>& tokens, size_t first, int lineno,
                        std::vector<Expr>& args);

    // Expressions, both passes
    bool evalExpr(const Expr& expr, uint16_t pc, int lineno, bool final,
                  ExprResult& result);
    bool evalSum(const Expr& expr, size_t& i, uint16_t pc, int lineno, bool final,
                 long& value, bool& known);
    bool evalTerm(const Expr& expr, size_t& i, uint16_t pc, int lineno, bool final,
                  long& value, bool& known);

    // Pass 2: statements -> bytes, expressions resolved
    void emit();
    void writeListing();

    void error(int line, int col, const char* fmt, ...);

    const char*                     filename_;   // for error messages
    uint8_t*                        memory_;     // the emulator's 64K image
    FILE*                           listing_;    // where to write the listing, or NULL
    uint16_t                        pc_;         // location counter during pass 1
    int                             errors_;
    std::map<std::string, uint16_t> labels_;     // labels and constants, filled in pass 1
    std::vector<Statement>          statements_; // in source order, filled in pass 1
    std::vector<std::string>        source_;     // the file's lines, for the listing
    std::vector<LineInfo>           lines_;      // per source line, for the listing
};

Assembler::Assembler(const char* filename, uint8_t* memory, FILE* listing)
    : filename_(filename), memory_(memory), listing_(listing), pc_(0), errors_(0)
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
 *   '#'                  kHash (a one-character token)
 *   ( ) , : = + - < > *  kPunct (one-character tokens, so "($10),Y" and
 *                        "LABEL+1" lex without spaces)
 *   '$' '%' or digit     kNumber in hex, binary or decimal; the digits that
 *                        follow are validated against the radix here so
 *                        later stages can trust token.value
 *   'c'                  kNumber with the character's code
 *   "text"               kString, case preserved
 *   '.'                  kDirective
 *   letter or '_'        kIdentifier
 *   anything else        error
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

        if (c == '#')
        {
            token.kind = Token::kHash;
            token.text = "#";
            i++;
        }
        else if (strchr("(),:=+-<>*", c) != NULL)
        {
            token.kind = Token::kPunct;
            token.text = std::string(1, c);
            i++;
        }
        else if (c == '\'') // character literal
        {
            if (i + 2 >= len || line[i+2] != '\'')
            {
                error(lineno, token.col, "character literal must be one character in quotes");
                return false;
            }
            token.kind = Token::kNumber;
            token.radix = Token::kChar;
            token.value = (unsigned char)line[i+1];
            token.text = std::string(1, line[i+1]);
            i += 3;
        }
        else if (c == '"') // string
        {
            size_t end = i + 1;
            while (end < len && line[end] != '"') end++;
            if (end >= len)
            {
                error(lineno, token.col, "unterminated string");
                return false;
            }
            token.kind = Token::kString;
            token.text = std::string(line + i + 1, end - i - 1);
            i = end + 1;
        }
        else if (c == '$' || c == '%' || isdigit((unsigned char)c))
        {
            token.kind = Token::kNumber;
            token.radix = (c == '$') ? Token::kHex : (c == '%') ? Token::kBinary : Token::kDecimal;
            if (token.radix != Token::kDecimal) i++; // the $ or % is not part of the text

            // Collect every alphanumeric so that a malformed number such
            // as $12G4 is reported as one bad value rather than split in two.
            size_t start = i;
            while (i < len && isalnum((unsigned char)line[i]))
            {
                token.text += (char)toupper((unsigned char)line[i]);
                i++;
            }

            const char* digits = (token.radix == Token::kHex) ? "0123456789ABCDEF" :
                                 (token.radix == Token::kBinary) ? "01" : "0123456789";
            const char* name   = (token.radix == Token::kHex) ? "hex" :
                                 (token.radix == Token::kBinary) ? "binary" : "decimal";
            const char* prefix = (token.radix == Token::kHex) ? "$" :
                                 (token.radix == Token::kBinary) ? "%" : "";

            if (token.text.empty() ||
                strspn(token.text.c_str(), digits) != token.text.size())
            {
                error(lineno, (int)start + 1, "bad %s value ->%s%s<-",
                      name, prefix, token.text.c_str());
                return false;
            }

            // The parser checks digit counts (2 for a byte, 4 for an
            // address) and reports them; here just avoid overflowing on an
            // absurdly long literal.
            int base = (token.radix == Token::kHex) ? 16 : (token.radix == Token::kBinary) ? 2 : 10;
            if (token.text.size() <= 16)
            {
                token.value = strtoul(token.text.c_str(), NULL, base);
                if (token.value > 0xffffffff) token.value = 0xffffffff;
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
 * Record a label or constant. Returns false if the name is already taken.
 */
bool Assembler::defineLabel(const Token& name, uint16_t value, int lineno)
{
    if (labels_.find(name.text) != labels_.end())
    {
        error(lineno, name.col, "label %s is already defined", name.text.c_str());
        return false;
    }

    FTRACE("Assembler recording label: %s = %04x",
        __FILE__, __LINE__, name.text.c_str(), value);

    labels_[name.text] = value;
    return true;
}

/*
 * Expressions
 *
 *   sum  := term (('+' | '-') term)*
 *   term := '<' term | '>' term | '-' term | number | identifier | '*'
 *
 * Evaluation is 16-bit; results wrap. An identifier not yet in labels_
 * makes the result unknown; in pass 2 (final) that is an error instead.
 */

/**
 * Evaluate a whole expression and describe the result. Reports syntax
 * errors, and in pass 2 undefined labels. Returns false on error.
 */
bool Assembler::evalExpr(const Expr& expr, uint16_t pc, int lineno, bool final,
                         ExprResult& result)
{
    if (expr.empty())
    {
        error(lineno, 0, "expected an expression");
        return false;
    }

    size_t i = 0;
    long value = 0;
    bool known = true;

    if (!evalSum(expr, i, pc, lineno, final, value, known)) return false;

    if (i < expr.size())
    {
        error(lineno, expr[i].col, "unexpected ->%s<- in expression", expr[i].text.c_str());
        return false;
    }

    result.known = known;
    result.value = (uint16_t)(value & 0xffff);
    result.byteOp = (expr[0].isPunct('<') || expr[0].isPunct('>'));
    result.wideHex = (expr.size() == 1 && expr[0].isHex() && expr[0].text.size() == 4);

    return true;
}

bool Assembler::evalSum(const Expr& expr, size_t& i, uint16_t pc, int lineno, bool final,
                        long& value, bool& known)
{
    if (!evalTerm(expr, i, pc, lineno, final, value, known)) return false;

    while (i < expr.size() && (expr[i].isPunct('+') || expr[i].isPunct('-')))
    {
        bool add = expr[i].isPunct('+');
        i++;

        long rhs = 0;
        if (!evalTerm(expr, i, pc, lineno, final, rhs, known)) return false;

        value = add ? value + rhs : value - rhs;
    }

    return true;
}

bool Assembler::evalTerm(const Expr& expr, size_t& i, uint16_t pc, int lineno, bool final,
                         long& value, bool& known)
{
    if (i >= expr.size())
    {
        error(lineno, expr.back().col + (int)expr.back().text.size(),
              "expression ends with an operator");
        return false;
    }

    const Token& tok = expr[i];

    if (tok.isPunct('<') || tok.isPunct('>') || tok.isPunct('-'))
    {
        i++;
        if (!evalTerm(expr, i, pc, lineno, final, value, known)) return false;

        if (tok.isPunct('<'))      value = value & 0xff;
        else if (tok.isPunct('>')) value = (value >> 8) & 0xff;
        else                       value = -value;
        return true;
    }

    i++;

    if (tok.kind == Token::kNumber)
    {
        if (tok.value > 0xffff)
        {
            error(lineno, tok.col, "value out of range, ->%s<-", tok.text.c_str());
            return false;
        }
        value = (long)tok.value;
        return true;
    }

    if (tok.isPunct('*'))
    {
        value = pc;
        return true;
    }

    if (tok.kind == Token::kIdentifier)
    {
        std::map<std::string, uint16_t>::const_iterator it = labels_.find(tok.text);

        if (it != labels_.end())
        {
            value = it->second;
        }
        else if (final)
        {
            error(lineno, tok.col, "undefined label %s", tok.text.c_str());
            return false;
        }
        else
        {
            known = false;
            value = 0;
        }
        return true;
    }

    error(lineno, tok.col, "unexpected ->%s<- in expression", tok.text.c_str());
    return false;
}

/**
 * Split tokens[first..] at commas into argument expressions, for
 * directives. Reports empty arguments. Returns false on error.
 */
bool Assembler::splitArguments(const std::vector<Token>& tokens, size_t first, int lineno,
                               std::vector<Expr>& args)
{
    Expr current;

    for (size_t i = first; i <= tokens.size(); i++)
    {
        bool end = (i == tokens.size());

        if (end || tokens[i].isPunct(','))
        {
            if (current.empty())
            {
                error(lineno, end ? tokens.back().col : tokens[i].col, "expected a value");
                return false;
            }
            args.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(tokens[i]);
        }
    }

    return true;
}

/**
 * Read the operand tokens (tokens[first..]) into a Shape: which of the
 * 6502 operand patterns they form, and the expression inside it.
 *
 *   (nothing)          kNone
 *   A                  kAccumulator
 *   # e                kImmediate
 *   e                  kDirect
 *   e , X   /  e , Y   kDirectX / kDirectY
 *   ( e )              kIndirect
 *   ( e , X )          kIndirectX
 *   ( e ) , Y          kIndirectY
 *
 * where e is an expression: a run of tokens up to the next ')' or ','.
 * This is pure syntax: whether the instruction supports the pattern is
 * decided by encodeLegacy() or encodeStandard(), and the expression is
 * checked there. Returns false after reporting an error.
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

    shape.col = tokens[i].col;

    // Helpers expressed as small lambdas would be neater, but this file is
    // kept to the C++ the rest of the project uses.
    #define IS_PUNCT(idx, ch) ((idx) < n && tokens[idx].isPunct(ch))
    #define IS_REG(idx, name) ((idx) < n && tokens[idx].kind == Token::kIdentifier && tokens[idx].text == (name))
    #define TAKE_EXPR()       while (i < n && !tokens[i].isPunct(')') && !tokens[i].isPunct(',')) shape.expr.push_back(tokens[i++])

    if (IS_REG(i, "A") && i + 1 == n)
    {
        shape.kind = Shape::kAccumulator;
        return true;
    }

    if (tokens[i].kind == Token::kHash) // # e
    {
        i++;
        TAKE_EXPR();
        if (shape.expr.empty())
        {
            error(lineno, tokens[first].col, "expected a value after #");
            return false;
        }
        shape.kind = Shape::kImmediate;
    }
    else if (IS_PUNCT(i, '(')) // ( e ...
    {
        i++;
        TAKE_EXPR();
        if (shape.expr.empty())
        {
            error(lineno, tokens[first].col, "expected a value after (");
            return false;
        }

        if (IS_PUNCT(i, ',')) // ( e , X )
        {
            if (!IS_REG(i + 1, "X") || !IS_PUNCT(i + 2, ')'))
            {
                error(lineno, tokens[first].col, "expected (value,X)");
                return false;
            }
            shape.kind = Shape::kIndirectX;
            i += 3;
        }
        else if (IS_PUNCT(i, ')'))
        {
            if (IS_PUNCT(i + 1, ',')) // ( e ) , Y
            {
                if (!IS_REG(i + 2, "Y"))
                {
                    error(lineno, tokens[i + 1].col, "expected (value),Y");
                    return false;
                }
                shape.kind = Shape::kIndirectY;
                i += 3;
            }
            else // ( e )
            {
                shape.kind = Shape::kIndirect;
                i += 1;
            }
        }
        else
        {
            error(lineno, tokens[first].col, "unbalanced parenthesis in operand");
            return false;
        }
    }
    else if (!IS_PUNCT(i, ')') && !IS_PUNCT(i, ',')) // e [, X|Y]
    {
        TAKE_EXPR();
        shape.kind = Shape::kDirect;

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
    #undef TAKE_EXPR

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
 *                 filled in by emit(). Any other expression is treated the
 *                 same way.
 *
 * These are the rules the original assembler applied to its test programs,
 * kept so existing sources assemble to identical bytes. The standard forms
 * e,X  (e)  and so on are not accepted here: the suffix already said that.
 *
 * Returns false after reporting an error.
 */
bool Assembler::encodeLegacy(const Shape& shape, uint8_t opcode, int lineno,
                             Operand& operand)
{
    uint8_t bytes = asmInstructionBytes(opcode);

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
        error(lineno, shape.col, "instruction takes no operand");
        return false;
    }

    if (shape.kind != Shape::kImmediate && shape.kind != Shape::kDirect)
    {
        error(lineno, shape.col,
              "%s spells its addressing mode; use the bare mnemonic for standard syntax",
              asmInstructionSymbol(opcode));
        return false;
    }

    // Check the expression's syntax now, whatever its value
    ExprResult result;
    if (!evalExpr(shape.expr, pc_, lineno, false, result)) return false;

    operand.expr = shape.expr;
    operand.pc = pc_;
    operand.col = shape.col;
    operand.width = bytes - 1;
    operand.relative = isBranch(opcode);
    operand.immediate = (shape.kind == Shape::kImmediate);

    bool single = (shape.expr.size() == 1 && shape.expr[0].kind == Token::kNumber);
    const Token& tok = shape.expr[0];

    if (shape.kind == Shape::kImmediate)
    {
        if (single && tok.radix == Token::kHex && tok.text.size() > 2)
        {
            error(lineno, tok.col, "wrong number of digits in hex value, ->$%s<-",
                  tok.text.c_str());
            return false;
        }
        if (single && tok.radix == Token::kDecimal && tok.text.size() > 3)
        {
            error(lineno, tok.col, "wrong number of digits in decimal value, ->%s<-",
                  tok.text.c_str());
            return false;
        }
        if (single && tok.value > 0xff)
        {
            error(lineno, tok.col, "immediate value out of range, ->%s<-",
                  tok.text.c_str());
            return false;
        }

        operand.width = 1;
        return true;
    }

    // Direct: a label or other expression is sized by the instruction
    if (!single) return true;

    // Address literal
    if (tok.radix != Token::kHex)
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

    return true;
}

/**
 * Encode the operand for a bare mnemonic by choosing the addressing mode
 * from the operand's shape and value, then looking up (mnemonic, mode).
 *
 *   shape           candidate modes
 *   (none) / A      implied
 *   #e              immediate
 *   e               relative if the mnemonic is a branch; else zero page
 *                   or absolute
 *   e,X   e,Y       zero page,X or absolute,X;  zero page,Y or absolute,Y
 *   (e)             (indirect)
 *   (e,X)  (e),Y    (indirect,X)  (indirect),Y
 *
 * Zero page is chosen over absolute when the mnemonic has a zero page form
 * and the value is known to fit: a literal under $100 written with fewer
 * than four digits, an expression over labels already defined whose value
 * is under $100, or any <e or >e. A label defined later in the file is
 * assumed absolute, so that pass 1 can fix the instruction's size without
 * knowing the label. When only the zero page form exists (STX e,Y; every
 * (indirect,X) and (indirect),Y) the value must fit in a byte, which
 * emit() checks.
 *
 * Returns false after reporting an error.
 */
bool Assembler::encodeStandard(const Shape& shape, const std::string& mnemonic,
                               const OpcodeSet& set, int lineno, int col,
                               uint8_t& opcode, Operand& operand)
{
    ExprResult result;
    bool fitsZeroPage = false;

    if (shape.kind != Shape::kNone && shape.kind != Shape::kAccumulator)
    {
        if (!evalExpr(shape.expr, pc_, lineno, false, result)) return false;
        fitsZeroPage = result.byteOp || (result.known && result.value <= 0xff && !result.wideHex);
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
        error(lineno, shape.col ? shape.col : col, "%s has no %s form",
              mnemonic.c_str(), kModeNames[mode == kModeCount ? kImplied : mode]);
        return false;
    }

    opcode = (uint8_t)set.opcode[mode];
    operand.width = kModeWidth[mode];
    operand.relative = (mode == kRelative);
    operand.immediate = (mode == kImmediate);
    operand.col = shape.col;
    operand.pc = pc_;

    if (operand.width == 0) return true;

    operand.expr = shape.expr;

    // A value known now that cannot fit is reported here, where the mode
    // can be named; emit() repeats the check for values known only later.
    if (result.known && operand.width == 1 && !operand.relative && result.value > 0xff &&
        !(operand.immediate && result.value >= 0xff80))
    {
        error(lineno, shape.col, "%s %s takes a 1-byte value, got $%04x",
              mnemonic.c_str(), kModeNames[mode], result.value);
        return false;
    }

    return true;
}

/**
 * Parse the values of the legacy .DATA directive (tokens[first..]).
 *
 * Each value is 1-4 hex digits with an optional '$' prefix. A value above
 * $FF produces two bytes, low byte first; otherwise one byte. So
 * ".DATA $06 $1234" produces 06 34 12. Values are separated by spaces.
 *
 * Returns false after reporting an error.
 */
bool Assembler::parseDataLegacy(const std::vector<Token>& tokens, size_t first,
                                int lineno, std::vector<DataItem>& data)
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
        bool ok = ((tok.kind == Token::kNumber && tok.radix != Token::kChar) ||
                   tok.kind == Token::kIdentifier) &&
                  !tok.text.empty() && tok.text.size() <= 4 &&
                  strspn(tok.text.c_str(), "0123456789ABCDEF") == tok.text.size();

        if (!ok)
        {
            error(lineno, tok.col, "invalid hex value in data section, ->%s%s<-",
                  tok.isHex() ? "$" : "", tok.text.c_str());
            return false;
        }

        unsigned long value = strtoul(tok.text.c_str(), NULL, 16);

        DataItem item;
        item.expr = literalExpr(value, tok.col);
        item.width = (value > 0xff) ? 2 : 1;
        item.col = tok.col;
        data.push_back(item);
    }

    return true;
}

/**
 * Parse a directive (tokens[first] is the directive token).
 *
 *   .ORG e           sets pc_; e must be computable now. Not a statement.
 *   .BYTE e, ...     one DataItem of width 1 per expression; a "string"
 *                    argument gives one DataItem per character
 *   .WORD e, ...     one DataItem of width 2 per expression
 *   .TEXT "s", e...  the same as .BYTE; the name documents intent
 *   .DATA h h ...    legacy, see parseDataLegacy()
 *
 * .EQU is handled by parseLine(), since the name comes before it.
 *
 * On return isStatement says whether statement was filled in (.ORG only
 * moves pc_). Returns false after reporting an error.
 */
bool Assembler::parseDirective(const std::vector<Token>& tokens, size_t first,
                               int lineno, Statement& statement, bool& isStatement)
{
    const Token& dir = tokens[first];
    isStatement = false;

    if (dir.text == ".ORG")
    {
        Expr expr(tokens.begin() + first + 1, tokens.end());
        ExprResult result;

        if (expr.empty())
        {
            error(lineno, dir.col, ".ORG requires an address");
            return false;
        }
        if (!evalExpr(expr, pc_, lineno, false, result)) return false;
        if (!result.known)
        {
            error(lineno, expr[0].col, ".ORG address must not depend on a label defined later");
            return false;
        }

        pc_ = result.value;
        return true;
    }

    statement.kind = Statement::kData;
    isStatement = true;

    if (dir.text == ".DATA")
    {
        return parseDataLegacy(tokens, first + 1, lineno, statement.data);
    }

    if (dir.text != ".BYTE" && dir.text != ".WORD" && dir.text != ".TEXT")
    {
        error(lineno, dir.col, "unknown directive %s", dir.text.c_str());
        return false;
    }

    if (first + 1 >= tokens.size())
    {
        error(lineno, dir.col, "%s requires at least one value", dir.text.c_str());
        return false;
    }

    std::vector<Expr> args;
    if (!splitArguments(tokens, first + 1, lineno, args)) return false;

    for (size_t a = 0; a < args.size(); a++)
    {
        const Expr& expr = args[a];

        if (expr.size() == 1 && expr[0].kind == Token::kString)
        {
            if (dir.text == ".WORD")
            {
                error(lineno, expr[0].col, "strings are only allowed in .BYTE and .TEXT");
                return false;
            }

            const std::string& text = expr[0].text;
            for (size_t c = 0; c < text.size(); c++)
            {
                DataItem item;
                item.expr = literalExpr((unsigned char)text[c], expr[0].col);
                item.width = 1;
                item.col = expr[0].col;
                statement.data.push_back(item);
            }
            continue;
        }

        // Check the expression's syntax now; its value may come later
        ExprResult result;
        if (!evalExpr(expr, pc_, lineno, false, result)) return false;

        DataItem item;
        item.expr = expr;
        item.width = (dir.text == ".WORD") ? 2 : 1;
        item.col = expr[0].col;
        statement.data.push_back(item);
    }

    return true;
}

/**
 * Parser: turn one line's tokens into a Statement (pass 1).
 *
 * Works left to right through the line shape
 *
 *     [$addr] [label[:]] MNEMONIC [operand]
 *     [$addr] [label[:]] .DIRECTIVE arguments
 *     NAME = expression
 *     *    = expression
 *
 *   1. A leading $addr sets the location counter.
 *   2. "NAME = e" defines a constant (e must be computable now) and
 *      "* = e" sets the location counter; neither produces a statement.
 *   3. An identifier in column 1, or any identifier followed by ':',
 *      defines a label at the current location. Column 1 is the rule for
 *      the colon-less form so an indented mnemonic is never mistaken for a
 *      label and a label is never confused with an operand elsewhere on
 *      the line.
 *   4. What remains is a directive (parseDirective) or a mnemonic. The
 *      operand's shape is read first (parseOperandShape), then:
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
    if (tokens[0].isHex())
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

    // 2. Constant definition ("NAME = e" or "NAME .EQU e") or "* = e"
    if (i + 1 < n &&
        (tokens[i+1].isPunct('=') ||
         (tokens[i+1].kind == Token::kDirective && tokens[i+1].text == ".EQU")) &&
        (tokens[i].kind == Token::kIdentifier || tokens[i].isPunct('*')))
    {
        Expr expr(tokens.begin() + i + 2, tokens.end());
        ExprResult result;

        if (expr.empty())
        {
            error(lineno, tokens[i+1].col, "expected a value after =");
            return;
        }
        if (!evalExpr(expr, pc_, lineno, false, result)) return;
        if (!result.known)
        {
            error(lineno, expr[0].col, "%s must not depend on a label defined later",
                  tokens[i].isPunct('*') ? "the location counter" : "a constant");
            return;
        }

        if (tokens[i].isPunct('*'))
        {
            pc_ = result.value;
            lines_[lineno-1].setAddress(pc_);
        }
        else if (defineLabel(tokens[i], result.value, lineno))
        {
            lines_[lineno-1].isConstant = true;
            lines_[lineno-1].value = result.value;
        }
        return;
    }

    // 3. Optional label: an identifier in column 1, or one followed by ':'
    if (i < n && tokens[i].kind == Token::kIdentifier)
    {
        bool colon = (i + 1 < n && tokens[i+1].isPunct(':'));

        if (colon || tokens[i].col == 1)
        {
            if (!defineLabel(tokens[i], pc_, lineno)) return;
            lines_[lineno-1].setAddress(pc_);
            i += colon ? 2 : 1;
        }
    }

    if (i >= n) return; // just a location or a label on its own

    // 4. The statement itself
    Statement statement;
    statement.address = pc_;
    statement.line = lineno;
    statement.opcode = 0;
    lines_[lineno-1].setAddress(pc_);

    const Token& tok = tokens[i];

    if (tok.kind == Token::kDirective)
    {
        bool isStatement = false;
        if (!parseDirective(tokens, i, lineno, statement, isStatement)) return;
        if (!isStatement) // .ORG: show the new address
        {
            lines_[lineno-1].hasAddress = false;
            lines_[lineno-1].setAddress(pc_);
            return;
        }

        for (size_t d = 0; d < statement.data.size(); d++)
        {
            pc_ += statement.data[d].width;
        }
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
              tok.isHex() ? "$" : "", tok.text.c_str());
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
 * By now labels_ is complete, so every expression can be evaluated (an
 * undefined label is reported here). Then:
 *
 *   relative (branches)      one byte: the signed distance from the address
 *                            after the operand byte to the target, which
 *                            must be within -128..127
 *   one-byte operand/data    the value must fit in a byte; an immediate may
 *                            also be a negative byte, -128..-1
 *   two-byte operand/data    low byte first
 *
 * An unresolvable or out-of-range value is reported at the line and column
 * of the operand and left unwritten.
 */
void Assembler::emit()
{
    for (size_t s = 0; s < statements_.size(); s++)
    {
        const Statement& statement = statements_[s];
        uint16_t ip = statement.address;
        std::vector<uint8_t>& listed = lines_[statement.line-1].bytes;

        if (statement.kind == Statement::kData)
        {
            for (size_t d = 0; d < statement.data.size(); d++)
            {
                const DataItem& item = statement.data[d];
                ExprResult result;

                if (!evalExpr(item.expr, statement.address, statement.line, true, result))
                {
                    ip += item.width;
                    continue;
                }
                if (item.width == 1 && result.value > 0xff)
                {
                    error(statement.line, item.col,
                          "value $%04x does not fit in a byte", result.value);
                    ip += item.width;
                    continue;
                }

                listed.push_back(LOBYTE(result.value));
                memory_[ip++] = LOBYTE(result.value);
                if (item.width == 2)
                {
                    listed.push_back(HIBYTE(result.value));
                    memory_[ip++] = HIBYTE(result.value);
                }
            }
            continue;
        }

        FTRACE("Assembler storing instruction: %02x at %04x",
            __FILE__, __LINE__, statement.opcode, ip);

        listed.push_back(statement.opcode);
        memory_[ip++] = statement.opcode;

        const Operand& operand = statement.operand;

        if (operand.width == 0) continue;

        ExprResult result;
        if (!evalExpr(operand.expr, operand.pc, statement.line, true, result)) continue;

        uint16_t value = result.value;

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
            if (operand.immediate && value >= 0xff80)
            {
                value &= 0xff; // a negative byte such as #-1
            }
            else
            {
                error(statement.line, operand.col,
                      "value $%04x does not fit in a 1-byte operand", value);
                continue;
            }
        }

        listed.push_back(LOBYTE(value));
        memory_[ip++] = LOBYTE(value);
        if (operand.width == 2)
        {
            listed.push_back(HIBYTE(value));
            memory_[ip++] = HIBYTE(value);
        }
    }
}

/**
 * Write the listing: one row per source line, with the address the line
 * occupies (or =value for a constant), up to eight of the bytes it
 * generated, and the source text. A line with more bytes continues on
 * following rows that show only address and bytes.
 *
 *   4000  A9 10                     LDA #$10
 *   =0050                           BASE = $50
 *   4002  48 65 6C 6C 6F 00         MSG  .TEXT "Hello", 0
 */
void Assembler::writeListing()
{
    const size_t kBytesPerRow = 8;

    for (size_t l = 0; l < source_.size(); l++)
    {
        const LineInfo& info = lines_[l];
        char addr[8] = "";
        char bytes[kBytesPerRow * 3 + 1] = "";

        if (info.isConstant)       snprintf(addr, sizeof(addr), "=%04X", info.value);
        else if (info.hasAddress)  snprintf(addr, sizeof(addr), "%04X", info.address);

        size_t shown = 0;
        for (; shown < info.bytes.size() && shown < kBytesPerRow; shown++)
        {
            snprintf(bytes + shown * 3, 4, "%02X ", info.bytes[shown]);
        }
        if (shown) bytes[shown * 3 - 1] = '\0'; // drop the trailing space

        fprintf(listing_, "%-5s  %-23s  %s\n", addr, bytes, source_[l].c_str());

        // Continuation rows for long data lines
        while (shown < info.bytes.size())
        {
            size_t row = 0;
            for (; row < kBytesPerRow && shown + row < info.bytes.size(); row++)
            {
                snprintf(bytes + row * 3, 4, "%02X ", info.bytes[shown + row]);
            }
            bytes[row * 3 - 1] = '\0';
            fprintf(listing_, "%04X   %s\n", (uint16_t)(info.address + shown), bytes);
            shown += row;
        }
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

        // Keep the line, without its line ending, for the listing
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) len--;
        source_.push_back(std::string(line, len));
        lines_.push_back(LineInfo());

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

    if (listing_ != NULL) writeListing();

    return errors_;
}

} // namespace

/**
 * Entry point; see asm6502.h. assemble() in l6502.cpp calls this after
 * clearing the emulator's memory.
 */
int asmAssemble(const char* filename, uint8_t* memory, FILE* listing)
{
    Assembler assembler(filename, memory, listing);
    return assembler.run();
}
