/**
 * @section copyright_sec Copyright and License
 *
 * Copyright (c) 1998-2012 Jeff Budzinski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  
#include <ctype.h>
#include <string>
#include <map>

#include "l6502.h"
#include "ftrace.h"
#include "ticker.h"
#include "util.h"

/**
 * Version string
 */
const char* kVersion = "6502 Emulator v0.3.1";

/**
 * Consts for program stack size, label and branch tables, 
 * and instruction set mapping table.
 */
static const int kStackSize         = 256;
static const int kInstrSetTableSize = 256;

/**
 * Maximum input line length
 */
static const int kMaxLineLength = 1024;

/**
 * Consts used for manipulations of various 6502 status bits.
 */
static const uint8_t kCARRYBIT     = 0;
static const uint8_t kZEROBIT      = 1;
static const uint8_t kINTERRUPTBIT = 2;
static const uint8_t kDECIMALBIT   = 3;
static const uint8_t kBREAKBIT     = 4;
static const uint8_t kOVERFLOWBIT  = 6;
static const uint8_t kSIGNBIT      = 7;

/**
 * Byte manipulation macros
 */
#define HIBYTE(b) (b / 256)
#define LOBYTE(b) (b % 256)

/**
 * Flip bytes macros
 */
#define FLIP2(s) (((s>>8)&0xff)+((s&0xff00)<<8))
#define FLIP4(l) (((l>>24)&0xff)+((l>>8)&0xff00)+((l&0xff00)<<8)+((l&0xff)<<24))

/**
 * Macro to define an instruction's symbolic value, string, and 
 * function prototype
 */
#define INSTRUCTION(inst,opcode,size,cycles,desc) const char* s##inst = #inst; \
    const uint8_t inst = opcode; \
    const uint8_t inst##SZ = size; \
    const uint8_t inst##CYC = cycles; \
    const char* inst##DSC = desc; \
    void i##inst(void)

/*
 * Convenience macro that initializes the instruction table to
 * predictable values.
 */
#define MAP_INITIALIZE \
{\
    for (unsigned int ii=0; ii < kInstrSetTableSize; ii++) \
    { \
        i6502[ii].symbol="\0"; \
        i6502[ii].desc="\0"; \
        i6502[ii].bytes=0; \
        i6502[ii].cycles=0; \
        i6502[ii].pFunc=0; \
    } \
}

/**
 * Macro to fix up the instruction entry in the instruction table
 */
#define MAP_INSTRUCTION(inst) \
    i6502[inst].pFunc=&i##inst; \
    i6502[inst].bytes=inst##SZ; \
    i6502[inst].cycles=inst##CYC; \
    i6502[inst].symbol=s##inst; \
    i6502[inst].desc=inst##DSC;

/**
 * Macros to set the various 6502 status bits
 */
#define SET_ZERO(val) (ZERO = (val==0), P = (P&~(1<<kZEROBIT)) | (ZERO<<kZEROBIT))
#define SET_SIGN(val) (SIGN = (val&(1<<kSIGNBIT))==(1<<kSIGNBIT), P = (P&~(1<<kSIGNBIT)) | (SIGN<<kSIGNBIT))
#define SET_CARRY(val) (CARRY = (val==1), P = (P&~(1<<kCARRYBIT)) | (CARRY<<kCARRYBIT))
#define SET_OVERFLOW(val) (OVERFLOW = (val&(1<<kOVERFLOWBIT))==(1<<kOVERFLOWBIT), P = (P&~(1<<kOVERFLOWBIT)) | (OVERFLOW<<kOVERFLOWBIT))
#define SET_BREAK(val) (BREAK = val, P = (P&~(1<<kBREAKBIT)) | (BREAK<<kBREAKBIT))
#define SET_DECIMAL(val) (DECIMAL = val, P = (P&~(1<<kDECIMALBIT)) | (DECIMAL<<kDECIMALBIT))
#define SET_INTERRUPT(val) (INTERRUPT = val, P = (P&~(1<<kINTERRUPTBIT)) | (INTERRUPT<<kINTERRUPTBIT))

/**
 * Instruction descriptor
 */
typedef struct
{
    const char* symbol;  // Symbolic string for the instruction
    uint8_t bytes; // Instruction length - reserved for future use
    uint8_t cycles; // Number of CPU cycles for this instruction
    const char* desc; // Instruction description
    void (*pFunc)(void); // Function pointer for the instructions implementation
} INST_DESCRIPTOR;

/**
 * Association for symbols to addresses used by assembler.
 */
typedef std::map<std::string, uint16_t> SymbolAddressMap;

/**
 * Association for addresses to symbols used by assembler.
 */
typedef std::map<uint16_t, std::string> AddressSymbolMap;

/**
 * Association for active breakpoints.
 */
typedef std::map<uint16_t, bool> BreakpointMap;

/**
 * Initialization status
 */
static bool bInitialized = false;

/**
 * 6502 status "bits"
 */
static uint8_t CARRY;
static uint8_t ZERO;
static uint8_t INTERRUPT;
static uint8_t DECIMAL;
static uint8_t BREAK;
static uint8_t OVERFLOW;
static uint8_t SIGN;

static uint8_t* BP; /// Base address, not part of 6502

static uint8_t  A;  /// Accumulator
static uint8_t  X;  /// Index register X
static uint8_t  Y;  /// Index register Y
static uint16_t PC; /// Program counter
static uint8_t  SP; /// Stack pointer
static uint8_t  P;  /// Status register

/**
 * Program stack
 */
static uint8_t STACK[kStackSize]; 

/**
 * Program labels used by the assembler
 */
static SymbolAddressMap labels;

/**
 * Branches to labels used by the assembler
 */
static AddressSymbolMap branches;

//
// 6502 instruction set table (indexed by opcode)
//
static INST_DESCRIPTOR i6502[kInstrSetTableSize];

//
// 64k RAM for execution environment
//
static uint8_t memory[k64K];

/**
 * Breakpoints for debugging
 */
static BreakpointMap breakpoints;

/**
 * Debugger actions.
 */
typedef enum 
{
    kUnknown,
    kExit,
    kPrint,
    kFlags,
    kRegisters,
    kStack,
    kStep,
    kContinue,
    kBreak,
    kClear,
    kRun,
    kTrace, 
    kList, 
    kAssert,
    kHelp
} ACTION; 
 
/*
 * Map input a single debug command and shortcut to a program action.
 */
typedef struct
{
    const char*  command;
    const char*  abbrev;
    ACTION action;
} COMMAND_TO_ACTION;

/*
 * Maps known deubg command strings to actions.
 */
static COMMAND_TO_ACTION commands[] =
{
    {"EXIT", "X", kExit},
    {"QUIT", "Q", kExit},
    {"PRINT", "P", kPrint},
    {"FLAGS", "F", kFlags},
    {"REGISTERS", "E", kRegisters},
    {"STACK", "A", kStack},
    {"STEP", "S", kStep},
    {"BREAK", "B", kBreak},
    {"CLEAR", "C", kClear},
    {"GO", "G", kContinue},
    {"RUN", "R", kRun},
    {"TRACE", "T", kTrace},
    {"LIST", "L", kList},
    {"ASSERT", "A", kAssert},
    {"HELP", "H", kHelp}
};

/**
 * Convenience function to get an address from the PC's following bytes in 
 * the compiled object code.
 */ 
uint16_t getAbsoluteAddress()
{
    return (*(BP+PC+2)<<8) + *(BP+PC+1);
}

/**
 * Convenience function to get an address from the address found at the 
 * address specified in the compiled object code.
 */
uint16_t getIndirectAddress()
{
    uint16_t pc = getAbsoluteAddress();
    return (*(BP+pc+1)<<8) + *(BP+pc); 
}

/** 
 * Convenience function to calculate the new program counter value for a
 * branch instruction.
 */
uint16_t getRelativeAddress()
{
    uint16_t pc = PC;

    if (*(BP+pc+1) >= 0x80)
    {
        pc -= 0x100 - *(BP+PC+1);
        pc += 2;
    }
    else
    {
        pc += 2 + *(BP+pc+1);
    }

    return pc;
}

/**
 * Convenience function use to get an immediate value
 * after an instruction.
 */
unsigned char getImmediateValue()
{
    return *(BP+PC+1);
}

//
// The following section contains instruction implementation functions in
// roughly alphabetical order.
//

/**
 * Add immediate value to accumulator with carry
 */ 
INSTRUCTION(ADCI, 0x69, 2, 2, "Add with carry immediate")
{
    char value = getImmediateValue();
    FTRACE("%s %02x", __FILE__, __LINE__, sADCI, (uint8_t)value);
    uint16_t a = (uint16_t)A + value + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
    // !!! add decimal mode addition
}

/**
 * Add memory to accumulator with carry, zero page 
 */
INSTRUCTION(ADCZ, 0x65, 2, 3, "Add with carry from zero page address")
{
    char value = getImmediateValue();
    FTRACE("%s %02x", __FILE__, __LINE__, sADCZ, (uint8_t)value);
    uint16_t a = (uint16_t)A + *(BP+value) + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Add to accumulator from absolute address with carry
 */
INSTRUCTION(ADCA, 0x6D, 3, 4, "Add with carry from absolute address")
{
    uint16_t pc = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sADCA, (uint16_t)pc);
    uint16_t a = (uint16_t)A + *(BP + pc) + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * Add to accumulator from zero page address with carry
 */
INSTRUCTION(ADCZX, 0x61, 2, 6, "Add with carry from zero page indexed")
{
    char value = getImmediateValue();
    FTRACE("%s %02x", __FILE__, __LINE__, sADCZX, (uint16_t)value);
    uint8_t zx = value + X;
    uint16_t a = (uint16_t)A + *(BP + zx) + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Add to accumulator with carry from indirect address offset from X
 */
INSTRUCTION(ADCIX, 0x75, 2, 4, "Add with carry from indirect, X")
{
    char value = getImmediateValue();
    FTRACE("%s %02x", __FILE__, __LINE__, sADCIX, (uint8_t)value);
    uint8_t zx = value + X;
    uint16_t a = (uint16_t)A + *(BP + (*(BP + zx + 1)<<8) + *(BP + zx)) + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Add to accumulator with carry from indirect address offset from Y
 */
INSTRUCTION(ADCIY, 0x71, 2, 5, "Add with carry from indirect, Y")
{
    uint8_t zi = *(BP+PC+1);
    FTRACE("%s %02x", __FILE__, __LINE__, sADCIY, (uint8_t)zi);
    uint16_t a = (uint16_t)A + *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y) + CARRY;                 
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Add to accumulator with carry from absolute address and X
 */
INSTRUCTION(ADCX, 0x7D, 3, 4, "Add with carry from absolute, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sADCX, (uint16_t)addr16);
    uint16_t a = (uint16_t)A + *(BP + addr16 + X) + CARRY;
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * Add to accumulator with carry from absolute address and Y
 */
INSTRUCTION(ADCY, 0x79, 3, 4, "Add with carry from absolute, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sADCY, (uint16_t)addr16);
    uint16_t a = (uint16_t)A + *(BP + addr16 + Y) + CARRY;
    SET_CARRY((a > 0xff));
    A = (uint8_t)a;
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * AND the accumulator using immediate value
 */
INSTRUCTION(ANDI, 0x29, 2, 2, "AND with immediate value")
{
    char value = getImmediateValue();
    FTRACE("%s %02x", __FILE__, __LINE__, sANDI, (uint8_t)value);
    A &= value;
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * AND the accumulator with value from zero page memory
 */
INSTRUCTION(ANDZ, 0x25, 2, 3, "AND from zero page memory address")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sANDZ, (uint8_t)*(BP+PC+1));
    A &= *(BP+*(BP+PC+1));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * AND the accumulator with value from absolute memory address
 */
INSTRUCTION(ANDA, 0x2D, 3, 4, "AND from absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sANDA, (uint16_t)addr16);
    A &= *(BP + addr16);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * AND the accumulator with value from memory address indexed by zero page
 * value plus X
 */
INSTRUCTION(ANDZX, 0x21, 2, 6, "AND from zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sANDZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A &= *(BP + zx);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * AND the accumulator with value from memory address found using absolute 
 * address plus X
 */
INSTRUCTION(ANDX, 0x3D, 3, 4, "AND from absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sANDX, (uint16_t)addr16);
    A &= *(BP + addr16 + X);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * AND the accumulator with the value from the memory address found using
 * the absolute address plus Y
 */
INSTRUCTION(ANDY, 0x39, 3, 4, "AND from absolute address, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sANDY, (uint16_t)addr16);
    A &= *(BP + addr16 + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * AND the accumulator with value from memory address indexed by immediate
 * value plus X (indexed indirect addressing mode)
 */
INSTRUCTION(ANDIX, 0x35, 2, 4, "AND from indirect address, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sANDIX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap around
    A &= *(BP + (*(BP + zx + 1)<<8) + *(BP + zx));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * AND accumulator with value from memory address indexed by immediate
 * value plus Y (indirect indexed addressing mode)
 */
INSTRUCTION(ANDIY, 0x31, 2, 5, "AND from indirect address, Y")
{
    uint8_t zi = *(BP+PC+1);
    FTRACE("%s %02x", __FILE__, __LINE__, sANDIY, (uint8_t)zi);
    A &= *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Arithmetic shift left from accumulator
 */
INSTRUCTION(ASL, 0x0A, 1, 2, "Arithmetic shift left")
{
    FTRACE("%s", __FILE__, __LINE__, sASL);
    SET_CARRY(((A&0x80)==0x80));
    A = A<<1;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Arithmetic shift left from zero page address
 */
INSTRUCTION(ASLZ, 0x06, 2, 5, "Arithmetic shift left zero page address")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sASLZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Arithmetic shift left from absolute address
 */
INSTRUCTION(ASLA, 0x0E, 3, 6, "Arithmetic shift left absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sASLA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Arithmetic shift left from memory indexed by zero page plus
 * X
 */
INSTRUCTION(ASLZX, 0x16, 2, 6, "Arithmetic shift left zero page address, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sASLZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    uint8_t* addr = BP + *(BP+zx) + X;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Arithmetic shift left from memory indexed by absolute address
 * plus X
 */
INSTRUCTION(ASLX, 0x1E, 3, 7, "Arithmetic shift left absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sASLX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Test zero page value with accumulator
 */
INSTRUCTION(BITZ, 0x24, 2, 3, "Test accumulator with zero page address")
{
    uint8_t zi = *(BP+PC+1);
    FTRACE("%s %02x", __FILE__, __LINE__, sBITZ, (uint8_t)zi);
    SET_ZERO((A&*(BP+zi)));                 
    SET_SIGN((A&*(BP+zi)));
    SET_OVERFLOW(*(BP+zi));
    PC += 2;
}

/**
 * Test absolute addressed memory value with accumulator
 */
INSTRUCTION(BIT, 0x2C, 3, 4, "Test accumulator with absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sBIT, (uint16_t)addr16);
    SET_ZERO((A&*(BP + addr16)));
    SET_SIGN((A&*(BP + addr16)));
    SET_OVERFLOW(*(BP + addr16));
    PC += 3;
}

/**
 * Branch on carry clear
 */
INSTRUCTION(BCC, 0x90, 2, 2, "Branch to relative address on carry clear")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBCC, (uint8_t)*(BP+PC+1));
    if (CARRY == 0)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

/**
 * Branch on carry set
 */
INSTRUCTION(BCS, 0xB0, 2, 2, "Branch to relative address on carry set")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBCS, (uint8_t)*(BP+PC+1));
    if (CARRY == 1)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

// 
// Branch on overflow clear
//
INSTRUCTION(BVC, 0x50, 2, 2, "Branch to relative address on overflow clear")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBVC, (uint8_t)*(BP+PC+1));
    if (OVERFLOW == 0)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

/**
 * Branch on overflow set
 */
INSTRUCTION(BVS, 0x70, 2, 2, "Branch to relative address on overflow set")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBVS, (uint8_t)*(BP+PC+1));
    if (OVERFLOW == 1)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

/**
 * Branch on equal (zero bit set)
 */
INSTRUCTION(BEQ, 0xF0, 2, 2, "Branch to relative address on zero bit set")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBEQ, (uint8_t)*(BP+PC+1));
    if (ZERO == 1)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

/**
 * Branch on result not zero (zero bit clear)
 */
INSTRUCTION(BNE, 0xD0, 2, 2, "Branch to relative address on zero bit clear")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBNE, (uint8_t)*(BP+PC+1));
    if (ZERO == 0)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC += 2;
    }
}

/**
 * Branch on result plus (sign bit clear)
 */
INSTRUCTION(BPL, 0x10, 2, 2, "Branch to relative address on sign bit clear")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBPL, (uint8_t)*(BP+PC+1));
    if (SIGN == 0)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC += 2;
    }
}

/**
 * Branch on result minus (sign bit set)
 */
INSTRUCTION(BMI, 0x30, 2, 2, "Branch to relative address on sign bit set")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sBMI, (uint8_t)*(BP+PC+1));
    if (SIGN == 1)
    {
        PC = getRelativeAddress();
    }
    else
    {
        PC+=2;
    }
}

/**
 * Force break
 */
INSTRUCTION(BRK, 0x00, 1, 7, "Set break")
{
    FTRACE("%s", __FILE__, __LINE__, sBRK);
    SET_BREAK(1);
    // @todo correct the implementation of this instruction, 
    // see http://nesdev.parodius.com/the%20'B'%20flag%20&%20BRK%20instruction.txt
}

/**
 * Clear carry bit
 */
INSTRUCTION(CLC, 0x18, 1, 2, "Clear carry bit")
{
    FTRACE("%s", __FILE__, __LINE__, sCLC);
    SET_CARRY(0);
    PC++;
}

/**
 * Clear decimal bit
 */
INSTRUCTION(CLD, 0xD8, 1, 2, "Clear decimal bit")
{
    FTRACE("%s", __FILE__, __LINE__, sCLD);
    SET_DECIMAL(0);
    PC++;
}

/**
 * Clear interrupt bit
 */
INSTRUCTION(CLI, 0x58, 1, 2, "Clear interrupt bit")
{
    FTRACE("%s", __FILE__, __LINE__, sCLI);
    SET_INTERRUPT(0);
    PC++;
}

/**
 * Clear overflow bit
 */
INSTRUCTION(CLV, 0xB8, 1, 2, "Clear overflow bit")
{
    FTRACE("%s", __FILE__, __LINE__, sCLV);
    SET_OVERFLOW(0);
    PC++;
}

/**
 * Compare immediate value.
 */
INSTRUCTION(CMPI, 0xC9, 2, 2, "Compare immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCMPI, (uint8_t)*(BP+PC+1));
    uint8_t a = A - *(BP+PC+1);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 2;
}

/**
 * Compare zero page memory.
 */
INSTRUCTION(CMPZ, 0xC5, 2, 3, "Compare zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCMPZ, (uint8_t)*(BP+PC+1));
    uint8_t a = A - *(BP+*(BP+PC+1));
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 2;
}

/**
 * Compare memory found at absolute address
 */
INSTRUCTION(CMPA, 0xCD, 3, 4, "Compare memory using absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sCMPA, (uint16_t)addr16);
    uint8_t a = A + *(BP + addr16);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 3;
}

/**
 * Compare memory using zero page, X addressing mode
 */
INSTRUCTION(CMPZX, 0xC1, 2, 6, "Compare memory using zero page, X addressing mode")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCMPZX,*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    uint8_t a = A - *(BP + zx);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 2;
}

/**
 * Compare memory using absolute, X addressing mode.
 */
INSTRUCTION(CMPX, 0xFD, 3, 4, "Compare memory using absolute, X addressing mode")
{
    uint16_t addr16 = getAbsoluteAddress();	
    FTRACE("%s %04x", __FILE__, __LINE__, sCMPX, (uint16_t)addr16);
    uint8_t a = A - *(BP + addr16 + X);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 3;
}

/**
 * Compare memory using absolute, Y addressing mode
 */
INSTRUCTION(CMPY, 0xD9, 3, 4, "Compare memory using absolute, Y addressing mode")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sCMPY, (uint16_t)addr16);
    uint8_t a = A - *(BP + addr16 + Y);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 3;
}

/**
 * Compare memory using indexed indirect addressing mode (see 
 * http://www.obelisk.demon.co.uk/6502/addressing.html for modes)
 */
INSTRUCTION(CMPIX, 0xD5, 2, 4, "Compare memory using indexed indirect addressing mode")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCMPIX,*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    uint8_t a = A - *(BP + (*(BP + zx + 1)<<8) + *(BP + zx));
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 2;
}

/** 
 * Compare memory using indirect indexed addressing mode (see
 * http://www.obelisk.demon.co.uk/6502/addressing.html for modes)
 */
INSTRUCTION(CMPIY, 0xF1, 2, 5, "Compare memory using indirect indexed addressing mode")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCMPIY, (uint8_t)*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    uint8_t a = A - *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y);
    SET_CARRY(((a&0x80)==0x80));
    SET_ZERO(a);
    SET_SIGN(a);
    PC += 2;
}

/**
 * Compare X with immediate value
 */
INSTRUCTION(CPXI, 0xE0, 2, 2, "Compare X with immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCPXI, (uint8_t)*(BP+PC+1));
    uint8_t x = X - *(BP+PC+1);
    SET_CARRY(((x&0x80)==0x80));
    SET_ZERO(x);
    SET_SIGN(x);
    PC += 2;
}

/**
 * Compare X with zero page value
 */
INSTRUCTION(CPXZ, 0xE4, 2, 3, "Compare X with zero page value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCPXZ, (uint8_t)*(BP+PC+1));
    uint8_t x = X - *(BP+*(BP+PC+1));
    SET_CARRY(((x&0x80)==0x80));
    SET_ZERO(x);
    SET_SIGN(x);
    PC += 2;
}

/**
 * Compare X with absolute address memory
 */
INSTRUCTION(CPXA, 0xEC, 3, 4, "Compare X with absolute address memory")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sCPXA, (uint16_t)addr16);
    uint8_t x = X + *(BP + addr16);
    SET_CARRY(((x&0x80)==0x80));
    SET_ZERO(x);
    SET_SIGN(x);
    PC += 3;
}

/**
 * Compare Y with immediate value
 */
INSTRUCTION(CPYI, 0xC0, 2, 2, "Compare Y with immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCPYI, (uint8_t)*(BP+PC+1));
    uint8_t y = Y - *(BP+PC+1);
    SET_CARRY(((y&0x80)==0x80));
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 2;
}

/**
 * Compare Y with zero page memory
 */
INSTRUCTION(CPYZ, 0xC4, 2, 3, "Compare Y with zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sCPYZ, (uint8_t)*(BP+PC+1));
    uint8_t y = Y - *(BP+*(BP+PC+1)) - (1 - CARRY);
    SET_CARRY(((y&0x80)==0x80));
    SET_ZERO(y);
    SET_SIGN(y);
    PC += 2;
}

/**
 * Compare Y with absolute address memory
 */
INSTRUCTION(CPYA, 0xCC, 3, 4, "Compare Y with absolute address memory")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sCPYA, (uint16_t)addr16);
    uint8_t y = Y + *(BP + addr16);
    SET_CARRY(((y&0x80)==0x80));
    SET_ZERO(y);
    SET_SIGN(y);
    PC += 3;
}

/**
 * Decrement zero page memory address
 */
INSTRUCTION(DECZ, 0xC6, 2, 5, "Decrement zero page memory address")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sDECZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    *(addr) -= 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/** 
 * Decrement memory value at absolute address
 */
INSTRUCTION(DECA, 0xCE, 3, 6, "Decrement memory value at absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sDECA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    *(addr) -= 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Decrement memory value at address found by adding zero page memory
 * value and X
 */
INSTRUCTION(DECZX, 0xD6, 2, 6, "Decrement memory using zero page, X addressing")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sDECZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap around
    uint8_t* addr = BP + zx;
    *(addr) -= 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Decrement memory value at absolute address found by adding absolute 
 * address and X
 */
INSTRUCTION(DECX, 0xDE, 3, 7, "Decrement memory value at absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sDECX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    *(addr) -= 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Decrement X register
 */
INSTRUCTION(DEX, 0xCA, 1, 2, "Decrement X register")
{
    FTRACE("%s", __FILE__, __LINE__, sDEX);
    X--;
    SET_ZERO(X);
    SET_SIGN(X);
    PC++;
}

/**
 * Decrement Y
 */
INSTRUCTION(DEY, 0x88, 1, 2, "Decrement Y register")
{
    FTRACE("%s", __FILE__, __LINE__, sDEY);
    Y--;
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC++;
}

/**
 * Exclusive OR accumulator with immediate value
 */
INSTRUCTION(EORI, 0x49, 2, 2, "Exclusive OR accumulator with immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sEORI, (uint8_t)*(BP+PC+1));
    A = ~((~A)|*(BP+PC+1));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Exclusive OR accumulator with zero page memory
 */
INSTRUCTION(EORZ, 0x45, 2, 3, "Exclusive OR accumulator with zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sEORZ, (uint8_t)*(BP+PC+1));
    A = ~((~A)|*(BP+*(BP+PC+1)));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Exclusive OR accumulator with absolute memory
 */
INSTRUCTION(EORA, 0x4D, 3, 4, "Exclusive OR accumulator with absolute memory")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sEORA, (uint16_t)addr16);
    A = ~((~A)|*(BP + addr16));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/** 
 * Eclusive OR the accumulator with value at the address found using the 
 * zero page index value plus X
 */
INSTRUCTION(EORZX, 0x55, 2, 4, "Exclusive OR memory location at zero page address plus X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sEORZX,*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A = ~((~A)|*(BP + zx));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Exclusive OR the accumulator with the absolute address plus X
 */
INSTRUCTION(EORX, 0x5D, 3, 4, "Exclusive OR the accumulator with the absolute address plus X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sEORX, (uint16_t)addr16);
    A = ~((~A)|*(BP + addr16 + X));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * Exclusive OR the accumulator with the absolute address plus Y
 */
INSTRUCTION(EORY, 0x59, 3, 4, "Exclusive OR the accumulator with the absolute address plus Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sEORY, (uint16_t)addr16);
    A = ~((~A)|*(BP + addr16 + Y));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * Exclusive OR the accumulator with value from memory address indexed by 
 * immediate value plus X (indexed indirect addressing mode)
 */
INSTRUCTION(EORIX, 0x41, 2, 6, "Exclusive OR using indexed indirect addressing mode")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sEORIX,*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A = ~((~A)|*(BP + (*(BP + zx + 1)<<8) + *(BP + zx)));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Exclusive OR the accumulator with value from memory address indexed by 
 * immediate value plus Y (indirect indexed addressing mode)
 */
INSTRUCTION(EORIY, 0x51, 2, 5, "Exclusive OR using indirect indexed addressing mode")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sEORIY,*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    A = ~((~A)|*(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Increment memory value at absolute address
 */
INSTRUCTION(INCA, 0xEE, 3, 6, "Increment memory value at absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sINCA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    *(addr) += 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Increment X
 */
INSTRUCTION(INX, 0xE8, 1, 2, "Increment X register")
{
    FTRACE("%s", __FILE__, __LINE__, sINX);
    X++;
    SET_ZERO(X);
    SET_SIGN(X);
    PC++;
}

/**
 * Increment Y
 */
INSTRUCTION(INY, 0xC8, 1, 2, "Increment Y regsiter")
{
    FTRACE("%s", __FILE__, __LINE__, sINY);
    Y++;
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC++;
}

/**
 * Increment zero page memory address
 */
INSTRUCTION(INCZ, 0xE6, 2, 5, "Increment zero page memory address")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sINCZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    *(addr) += 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}


/**
 * Increment memory value at address found by adding zero page memory
 * value and X
 */
INSTRUCTION(INCZX, 0xF6, 2, 6, "Increment memory at zero page plus X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sINCZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap around
    uint8_t* addr = BP + zx;
    *(addr) += 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Increment memory at address found by adding absolute address
 * to X
 */
INSTRUCTION(INCX, 0xFE, 3, 7, "Increment memory at address found by adding absolute address to X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sINCX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    *(addr) += 1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Jump absolute
 */
INSTRUCTION(JMP, 0x4C, 3, 3, "Jump to absolute address")
{
    PC = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sJMP,PC); 
}

/**
 * Jump indirect
 */
INSTRUCTION(JMPI, 0x6C, 3, 5, "Jump to indirect address")
{
    PC = getIndirectAddress(); // xfer control to addr found there
    FTRACE("%s %04x", __FILE__, __LINE__, sJMPI,PC);
}

/**
 * Jump to sub-routine
 */
INSTRUCTION(JSR, 0x20, 3, 6, "Jump to subroutine")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sJSR, (uint16_t)addr16);
    STACK[SP] = (PC+2)>>8; 
    STACK[SP-1] = (PC+2)&0xFF; 
    SP -= 2;
    PC = addr16;
}

/**
 * Load accumulator from immediate value
 */
INSTRUCTION(LDAI, 0xa9, 2, 2, "Load accumulator with immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDAI, (uint8_t)*(BP+PC+1));
    A = *(BP+PC+1);                 
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Load accumulator from zero page memory
 */
INSTRUCTION(LDAZ, 0xa5, 2, 3, "Load accumulator from zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDAZ, (uint8_t)*(BP+PC+1));
    A = *(BP+*(BP+PC+1));                 
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Load accumulator from absolute address memory
 */
INSTRUCTION(LDAA, 0xAD, 3, 4, "Load accumulator from absolute address memory")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDAA, (uint16_t)addr16);
    A = *(BP + addr16);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * Load accumulator from memory address found by adding zero page
 * memory value to X
 */
INSTRUCTION(LDAZX, 0xB5, 2, 4, "Load accumulator from zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDAZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap around
    A = *(BP + zx);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Load accumulator from indirect memory address found by adding 
 * immediate value to X
 */
INSTRUCTION(LDAIX, 0xA1, 2, 6, "Load accumulator from indirect address, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDAIX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap around
    A = *(BP + (*(BP + zx + 1)<<8) + *(BP + zx));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Load accumulator from indirect address, Y
 */
INSTRUCTION(LDAIY, 0xB1, 2, 5, "Load accumulator from indirect address, Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDAIY, (uint8_t)*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    A = *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Load accumulator from address found by adding X to the following
 * absolute address
 */
INSTRUCTION(LDAX, 0xBD, 3, 4, "Load accumulator from absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDAX, (uint16_t)addr16);
    A = *(BP + addr16 + X);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/*
 * Load accumulator from address found by adding Y to the following
 * absolute address
 */
INSTRUCTION(LDAY, 0xB9, 3, 4, "Load accumulator from absolute address, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDAY, (uint16_t)addr16);
    A = *(BP + addr16 + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * Load X from immediate value
 */
INSTRUCTION(LDXI, 0xA2, 2, 2, "Load X from immediate")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDXI, (uint8_t)*(BP+PC+1));
    X = *(BP+PC+1);                 
    SET_ZERO(X);
    SET_SIGN(X);
    PC += 2;
}

/**
 * Load X from zero page memory address
 */
INSTRUCTION(LDXZ, 0xA6, 2, 3, "Load X from zero page")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDXZ, (uint8_t)*(BP+PC+1));
    X = *(BP+*(BP+PC+1));                 
    SET_ZERO(X);
    SET_SIGN(X);
    PC += 2;
}

/**
 * Load X from memory indexed by zero page address plus Y
 */
INSTRUCTION(LDXZY, 0xB6, 2, 4, "Load X from zero page, Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDXZY, (uint8_t)*(BP+PC+1));
    uint8_t zy = *(BP+PC+1)+Y;
    X = *(BP + zy);
    SET_ZERO(X);
    SET_SIGN(X);
    PC += 2;
}

/**
 * Load X from absolute address
 */
INSTRUCTION(LDXA, 0xAE, 3, 4, "Load X from absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDXA, (uint16_t)addr16);
    X = *(BP + addr16);
    SET_ZERO(X);
    SET_SIGN(X);
    PC += 3;
}

/**
 * Load X from memory at absolute address plus Y
 */
INSTRUCTION(LDXY, 0xBE, 3, 4, "Load X from absolute address, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDXY, (uint16_t)addr16);
    X = *(BP + addr16 + Y);
    SET_ZERO(X);
    SET_SIGN(X);
    PC += 3;
}

/**
 * Load Y from immediate value
 */
INSTRUCTION(LDYI, 0xA0, 2, 2, "Load Y from immediate")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDYI, (uint8_t)*(BP+PC+1));
    Y = *(BP+PC+1);                 
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 2;
}

/**
 * Load Y from zero page memory address
 */
INSTRUCTION(LDYZ, 0xA4, 2, 3, "Load Y from zero page")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDYZ, (uint8_t)*(BP+PC+1));
    Y = *(BP+*(BP+PC+1));                 
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 2;
}

/**
 * Load Y from memory indexed by zero page address plus X
 */
INSTRUCTION(LDYZX, 0xB4, 2, 4, "Load Y from zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLDYZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap
    Y = *(BP + zx);
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 2;
}

/*
 * Load Y from absolute address
 */
INSTRUCTION(LDYA, 0xAC, 3, 4, "Load Y from absolute address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDYA, (uint16_t)addr16);
    Y = *(BP + addr16);
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 3;
}

/**
 * Load Y from memory at absolute address plus X
 */
INSTRUCTION(LDYX, 0xBC, 3, 4, "Load Y from absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLDYX, (uint16_t)addr16);
    Y = *(BP + addr16 + X);
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC += 3;
}

/**
 * Logical shift accumulator to the right 
 */
INSTRUCTION(LSR, 0x4A, 1, 2, "Logical shift right accumulator")
{
    FTRACE("%s", __FILE__, __LINE__, sLSR);
    SET_CARRY((A&0x01));
    A = A>>1;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Logical shift zero page memory to the right
 */
INSTRUCTION(LSRZ, 0x46, 2, 5, "Logical shift right zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLSRZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Logical shift absolute memory address value to the right
 */
INSTRUCTION(LSRA, 0x4E, 3, 6, "Logical shift right absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLSRA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Logical shift zero page memory indexed by X to the right
 */
INSTRUCTION(LSRZX, 0x56, 2, 6, "Logical shift right zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sLSRZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1) + X; // zero page wrap
    uint8_t* addr = BP + zx;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Logical shift absolute memory value indexed by X to the right
 */
INSTRUCTION(LSRX, 0x5E, 3, 7, "Logical shift right absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sLSRX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * No operation
 */
INSTRUCTION(NOP, 0xEA, 1, 2, "No operation")
{
    FTRACE("%s", __FILE__, __LINE__, sNOP);
    PC++;
}

/**
 * OR the accumulator with the immediate value
 */
INSTRUCTION(ORAI, 0x09, 2, 2, "Logical OR accumulator with immediate value")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sORAI, (uint8_t)*(BP+PC+1));
    A |= *(BP+PC+1);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * OR the accumulator with the value at the zero page address
 */
INSTRUCTION(ORAZ, 0x05, 2, 3, "Logical OR accumulator with zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sORAZ, (uint8_t)*(BP+PC+1));
    A |= *(BP+*(BP+PC+1));
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * OR the accumulator with the value at the absolute address
 */
INSTRUCTION(ORAA, 0x0D, 3, 4, "Logical OR accumulator with absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sORAA, (uint16_t)addr16);
    A |= *(BP + addr16);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * OR the accumulator with value at the address found using the zero 
 * page index value plus X
 */
INSTRUCTION(ORAZX, 0x15, 2, 4, "Logical OR accumulator with zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sORAZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A |= *(BP + zx);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * OR the accumulator with the absolute address plus X
 */
INSTRUCTION(ORAX, 0x1D, 3, 4, "Logical OR accumulator with absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sORAX, (uint16_t)addr16);
    A |= *(BP + addr16 + X);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * OR the accumulator with the absolute address plus Y
 */
INSTRUCTION(ORAY, 0x19, 3, 4, "Logical OR accumulator with absolute address, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sORAY, (uint16_t)addr16);
    A |= *(BP + addr16 + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 3;
}

/**
 * OR the accumulator with value from memory address indexed by immediate
 * value plus X (indexed indirect addressing mode)
 */
INSTRUCTION(ORAIX, 0x01, 2, 6, "Logical OR accumulator using indirect indexed, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sORAIX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A |= *(BP + (*(BP + zx + 1)<<8) + *(BP + zx));                 
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * OR the accumulator with value from memory address indexed by immediate
 * value plus Y (indirect indexed addressing mode)
 */
INSTRUCTION(ORAIY, 0x11, 2, 5, "Logical OR accumulator using indexed indirect, Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sORAIY, (uint8_t)*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    A |= *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y);
    SET_ZERO(A);
    SET_SIGN(A);
    PC += 2;
}

/**
 * Push accumulator on stack
 */
INSTRUCTION(PHA, 0x48, 1, 3, "Push accumulator onto stack")
{
    FTRACE("%s", __FILE__, __LINE__, sPHA);
    STACK[SP] = A;
    SP--;
    PC++;
}

/**
 * Pull accumulator from stack
 */
INSTRUCTION(PLA, 0x68, 1, 4, "Pull accumulator from stack")
{
    FTRACE("%s", __FILE__, __LINE__, sPLA);
    A = STACK[SP+1];
    SP++;
    PC++;
}

/**
 * Push processor status on stack
 */
INSTRUCTION(PHP, 0x08, 1, 3, "Push processor status on stack")
{
    FTRACE("%s", __FILE__, __LINE__, sPHP);
    STACK[SP] = P;
    SP--;
    PC++;
}

/**
 * Pull process status from stack
 */
INSTRUCTION(PLP, 0x28, 1, 4, "Pull process status from stack")
{
    FTRACE("%s", __FILE__, __LINE__, sPLP);
    P = STACK[SP+1];
    ZERO = (P&(1<<kZEROBIT)) == (1<<kZEROBIT);
    SIGN = (P&(1<<kSIGNBIT)) == (1<<kSIGNBIT);
    CARRY = (P&(1<<kCARRYBIT)) == (1<<kCARRYBIT);
    OVERFLOW = (P&(1<<kOVERFLOWBIT)) == (1<<kOVERFLOWBIT);
    DECIMAL = (P&(1<<kDECIMALBIT)) == (1<<kDECIMALBIT);
    BREAK = (P&(1<<kBREAKBIT)) == (1<<kBREAKBIT);
    SP++;
    PC++;
}

/**
 * Rotate accumulator one bit left
 */
INSTRUCTION(ROL, 0x2A, 1, 2, "Rotate accumulator one bit left")
{
    FTRACE("%s", __FILE__, __LINE__, sROL);
    uint8_t c = CARRY;
    SET_CARRY(((A&0x80)==0x80));
    A = A<<1;
    A |= c;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Rotate zero page memory one bit left
 */
INSTRUCTION(ROLZ, 0x26, 2, 5, "Rotate zero page memory one bit left")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sROLZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    uint8_t c = CARRY;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    *addr |= c;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Rotate absolute memory value left
 */
INSTRUCTION(ROLA, 0x2E, 3, 6, "Rotate absolute memory value left")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sROLA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    uint8_t c = CARRY;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    *addr |= c;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Rotate zero page indexed memory left
 */
INSTRUCTION(ROLZX, 0x36, 2, 6, "Rotate zero page indexed memory left")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sROLZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap
    uint8_t* addr = BP + *(BP+zx) + X;
    uint8_t c = CARRY;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    *addr |= c;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Rotate absolute memory value indexed by X to the left
 */
INSTRUCTION(ROLX, 0x3E, 3, 7, "Rotate absolute memory value indexed by X to the left")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sROLX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    uint8_t c = CARRY;
    SET_CARRY(((*addr&0x80)==0x80));
    *addr = *addr<<1;
    *addr |= c;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Rotate accumulator right
 */
INSTRUCTION(ROR, 0x6A, 1, 2, "Rotate accumulator right")
{
    FTRACE("%s", __FILE__, __LINE__, sROR);
    uint8_t c = CARRY;
    SET_CARRY((A&0x01));
    A = A>>1;
    A |= c<<7;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Rotate zero page memory value right
 */
INSTRUCTION(RORZ, 0x66, 2, 5, "Rotate zero page memory value right")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sRORZ, (uint8_t)*(BP+PC+1));
    uint8_t* addr = BP + *(BP+PC+1);
    uint8_t c = CARRY;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    *addr |= c<<7;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Rotate absolute memory address value right
 */
INSTRUCTION(RORA, 0x6E, 3, 6, "Rotate absolute memory address value right")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sRORA, (uint16_t)addr16);
    uint8_t* addr = BP + addr16;
    uint8_t c = CARRY;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    *addr |= c<<7;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Rotate zero page indexed memory address value right
 */
INSTRUCTION(RORZX, 0x76, 2, 6, "Rotate zero page indexed memory address value right")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sRORZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap
    uint8_t* addr = BP + zx; 
    uint8_t c = CARRY;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    *addr |= c<<7;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 2;
}

/**
 * Rotate absolute memory value indexed by X to the right
 */
INSTRUCTION(RORX, 0x7E, 3, 7, "Rotate absolute memory value indexed by X to the right")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sRORX, (uint16_t)addr16);
    uint8_t* addr = BP + addr16 + X;
    uint8_t c = CARRY;
    SET_CARRY((*addr&0x01));
    *addr = *addr>>1;
    *addr |= c<<7;
    SET_ZERO(*addr);
    SET_SIGN(*addr);
    PC += 3;
}

/**
 * Return from interrupt, restoring status bits
 */
INSTRUCTION(RTI, 0x40, 3, 6, "Return from interrupt, restoring status bits")
{
    FTRACE("%s", __FILE__, __LINE__, sRTI);
    P = STACK[SP+1];
    ZERO = (P&(1<<kZEROBIT)) == (1<<kZEROBIT);
    SIGN = (P&(1<<kSIGNBIT)) == (1<<kSIGNBIT);
    CARRY = (P&(1<<kCARRYBIT)) == (1<<kCARRYBIT);
    OVERFLOW = (P&(1<<kOVERFLOWBIT)) == (1<<kOVERFLOWBIT);
    DECIMAL = (P&(1<<kDECIMALBIT)) == (1<<kDECIMALBIT);
    BREAK = (P&(1<<kBREAKBIT)) == (1<<kBREAKBIT);
    PC = (uint16_t)(STACK[SP+3]<<8)+(uint16_t)STACK[SP+2] + 1;
    SP += 3;
}

/**
 * Return from subroutine
 */
INSTRUCTION(RTS, 0x60, 1, 6, "Return from subroutine")
{
    FTRACE("%s", __FILE__, __LINE__, sRTS);
    PC = (uint16_t)(STACK[SP+2]<<8)+(uint16_t)STACK[SP+1]+1;
    SP += 2;
}

/**
 * Subtract immediate value from accumulator with carry
 */
INSTRUCTION(SBCI, 0xE9, 2, 2, "Subtract immediate value from accumulator with carry")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSBCI, (uint8_t)*(BP+PC+1));
    A = A - *(BP+PC+1) - (1 - CARRY);                 
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
    // !!! add decimal mode addition
}

/**
 * Subtract memory from accumulator with carry, zero page 
 */
INSTRUCTION(SBCZ, 0xE5, 2, 3, "Subtract memory from accumulator with carry, zero page")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSBCZ, (uint8_t)*(BP+PC+1));
    A = A - *(BP+*(BP+PC+1)) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Subtract absolute memory from accumulator with carry
 */
INSTRUCTION(SBCA, 0xED, 3, 4, "Subtract absolute memory from accumulator with carry")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSBCA, (uint16_t)addr16);
    A = A - *(BP + addr16) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * Subtract zero page memory from accumulator with carry
 */
INSTRUCTION(SBCZX, 0xE1, 2, 6, "Subtract zero page memory from accumulator with carry")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSBCZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A = A - *(BP + zx) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Subtract the accumulator with value from memory address indexed by immediate
 * value plus X (indexed indirect addressing mode)
 */
INSTRUCTION(SBCIX, 0xF5, 2, 4, "Subtract with carry from indirect, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSBCIX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X;
    A = A - *(BP + (*(BP + zx + 1)<<8) + *(BP + zx)) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Subtract from accumulator from absolute adress and Y
 */
INSTRUCTION(SBCY, 0xF9, 3, 4, "Subtract with carry from absolute, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSBCY, (uint16_t)addr16);
    A = A - *(BP + addr16 + Y) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * Subtract X from accumulator
 */
INSTRUCTION(SBCX, 0xFD, 3, 4, "Subtract with carry from absolute, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSBCX, (uint16_t)addr16);
    A = A - *(BP + addr16 + X) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 3;
}

/**
 * Subtract indirect address from accumulator 
 */
INSTRUCTION(SBCIY, 0xF1, 2, 5, "Subtract with carry from indirect, Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSBCIY, (uint8_t)*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    A = A - *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y) - (1 - CARRY);
    SET_CARRY(((A&0x80)==0x80));
    SET_ZERO(A);
    SET_SIGN(A);
    SET_OVERFLOW(((((~CARRY)&SIGN)|(CARRY&(~SIGN)))<<kOVERFLOWBIT));
    PC += 2;
}

/**
 * Set decimal bit
 */
INSTRUCTION(SED, 0xF8, 1, 2, "Set decimal bit")
{
    FTRACE("%s", __FILE__, __LINE__, sSED);
    SET_DECIMAL(1);
    PC++;
}

/**
 * Set carry bit
 */
INSTRUCTION(SEC, 0x38, 1, 2, "Set carry bit")
{
    FTRACE("%s", __FILE__, __LINE__, sSEC);
    SET_CARRY(1);
    PC++;
}

/**
 * Set interrupt bit
 */
INSTRUCTION(SEI, 0x78, 1, 2, "Set interrupt bit")
{
    FTRACE("%s", __FILE__, __LINE__, sSEI);
    SET_INTERRUPT(0);
    PC++;
}

/**
 * Store accumulator to zero page memory
 */
INSTRUCTION(STAZ, 0x85, 2, 3, "Store accumulator to zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTAZ, (uint8_t)*(BP+PC+1));
    *(BP+*(BP+PC+1)) = A;                 
    PC += 2;
}  

/**
 * Store accumulator to absolute memory address
 */
INSTRUCTION(STAA, 0x8D, 3, 4, "Store accumulator to absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSTAA, (uint16_t)addr16);
    *(BP + addr16) = A;
    PC += 3;
}

/**
 * Store accumulator to zero page memory address indexed
 * by X
 */
INSTRUCTION(STAZX, 0x95, 2, 4, "Store accumulator to zero page, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTAZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap
    *(BP + zx) = A;
    PC += 2;
}

/**
 * Store accumulator to memory address found by adding absolute address
 * to X
 */
INSTRUCTION(STAX, 0x9D, 3, 5, "Store accumulator to absolute address, X")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSTAX, (uint16_t)addr16);
    *(BP + addr16 + X) = A;
    PC += 3;
}

/**
 * Store accumulator to memory address found by adding absolute address
 * to Y
 */
INSTRUCTION(STAY, 0x99, 3, 5, "Store accumulator to absolute address, Y")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSTAY, (uint16_t)addr16);
    *(BP + addr16 + Y) = A;
    PC += 3;
}

/**
 * Store accumulator to indirect memory address found by adding zero
 * page value and X (indexed indirect)
 */
INSTRUCTION(STAIX, 0x81, 2, 6, "Store accumulator to indirect address, X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTAIX, (uint16_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1)+X; // zero page wrap
    *(BP + (*(BP + zx + 1)<<8) + *(BP + zx)) = A;
    PC += 2;
}

/** 
 * Store accumulator to indirect memory address found by adding zero 
 * page address value and Y (indirect indexed)
 */
INSTRUCTION(STAIY, 0x91, 2, 6, "Store accumulator to indirect address, Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTAIY, (uint16_t)*(BP+PC+1));
    uint8_t zi = *(BP+PC+1);
    *(BP + (*(BP+zi+1)<<8) + *(BP+zi) + Y) = A;
    PC += 2;
}

/**
 * Store X to zero page memory 
 */
INSTRUCTION(STXZ, 0x86, 2, 3, "Store X to zero page memory")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTXZ, (uint16_t)*(BP+PC+1));
    *(BP+*(BP+PC+1)) = X;                 
    PC += 2;
}

/**
 * Store X to absolute memory address
 */
INSTRUCTION(STXA, 0x8E, 3, 4, "Store X to absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSTXA, (uint16_t)addr16);
    *(BP + addr16) = X;
    PC += 3;
}

/**
 * Store X to zero page memory address indexed by Y
 */
INSTRUCTION(STXZY, 0x96, 2, 4, "Store X to memory indexed by zero page address plus Y")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTXZY, (uint8_t)*(BP+PC+1));
    uint8_t zy = *(BP+PC+1)+Y;
    *(BP + zy) = X;
    PC += 2;
}

/**
 * Store Y to zero page memory address
 */
INSTRUCTION(STYZ, 0x84, 2, 3, "Store Y to zero page memory address")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTYZ, (uint8_t)*(BP+PC+1));
    *(BP+*(BP+PC+1)) = Y;                 
    PC += 2;
}

/**
 * Store Y to absolute memory address
 */
INSTRUCTION(STYA, 0x8C, 3, 4, "Store Y to absolute memory address")
{
    uint16_t addr16 = getAbsoluteAddress();
    FTRACE("%s %04x", __FILE__, __LINE__, sSTYA, (uint8_t)addr16);
    *(BP + addr16) = Y;
    PC += 3;
}

/**
 * Store Y to zero page memory address indexed by X
 */
INSTRUCTION(STYZX, 0x94, 2, 4, "Store Y to zero page memory address indexed by X")
{
    FTRACE("%s %02x", __FILE__, __LINE__, sSTYZX, (uint8_t)*(BP+PC+1));
    uint8_t zx = *(BP+PC+1) + X; // zero page wrap
    *(BP + zx) = Y;
    PC += 2;
}

/**
 * Transfer accumulator to X
 */
INSTRUCTION(TAX, 0xAA, 1, 2, "Transfer accumulator to X")
{
    FTRACE("%s", __FILE__, __LINE__, sTAX);
    X = A;
    SET_ZERO(X);
    SET_SIGN(X);
    PC++;
}

/**
 * Transfer accumulator to Y
 */
INSTRUCTION(TAY, 0xA8, 1, 2, "Transfer accumulator to Y")
{
    FTRACE("%s", __FILE__, __LINE__, sTAY);
    Y = A;
    SET_ZERO(Y);
    SET_SIGN(Y);
    PC++;
}

/**
 * Transfer stack pointer to X
 */
INSTRUCTION(TSX, 0xBA, 1, 2, "Transfer stack pointer to X")
{
    FTRACE("%s", __FILE__, __LINE__, sTSX);
    X = SP;
    SET_ZERO(X);
    SET_SIGN(X);
    PC++;
}

/**
 * Transfer X to accumulator
 */
INSTRUCTION(TXA, 0x8A, 1, 2, "Transfer X to accumulator")
{
    FTRACE("%s", __FILE__, __LINE__, sTXA);
    A = X;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Transfer Y to accumulator
 */
INSTRUCTION(TYA, 0x98, 1, 2, "Transfer Y to accumulator")
{
    FTRACE("%s", __FILE__, __LINE__, sTYA);
    A = Y;
    SET_ZERO(A);
    SET_SIGN(A);
    PC++;
}

/**
 * Transfer X to stack pointer
 */
INSTRUCTION(TXS, 0x9A, 1, 2, "Transfer X to stack pointer")
{
    FTRACE("%s", __FILE__, __LINE__, sTXS);
    SP = X;
    PC++;
}

/*
 * Assert value at given address.
 */
bool assertmem(uint16_t address, uint8_t value)
{
    return memory[address] == value;
}

/**
 * Return the value of the carry flag.
 */
uint8_t carry()
{
    return CARRY;
}

/**
 * Return the value of the zero flag.
 */
uint8_t zero()
{
    return ZERO;
}

/**
 * Return the value of the interrupt flag.
 */
uint8_t interrupt()
{
    return INTERRUPT;
}

/**
 * Return the value of the decimal flag.
 */
uint8_t decimal()
{
    return DECIMAL;
}

/** 
 * Return the value of the break flag.
 */
uint8_t brk()
{
    return BREAK;
}

/**
 * Return the value of the overflow flag.
 */
uint8_t overflow()
{
    return OVERFLOW;
}

/**
 * Return the value of the sign flag.
 */
uint8_t sign()
{
    return SIGN;
}

/**
 * Return the value of the accumulator.
 */
uint8_t a()
{
    return A;
}

/**
 * Return the value of the X register.
 */
uint8_t x()
{
    return X;
}

/**
 * Return the value of the Y register.
 */
uint8_t y()
{
    return Y;
}

/**
 * Return the value of the program counter.
 */
uint16_t pc()
{
    return PC;
}

/**
 * Return the value of the stack pointer.
 */
uint8_t sp()
{
    return SP;
}

/**
 * Return the value of the status register.
 */
uint8_t p()
{
    return P;
}

/**
 * Return the next input token from the sequence.
 */
char* getToken(char* token, char** tokens)
{
    int i = 0;

    assert(token);
    assert(tokens);

    while (**tokens != '\0' && isspace(**tokens)) (*tokens)++;
    while (**tokens != '\0' && !isspace(**tokens))
    {
        token[i++] = **tokens;
        (*tokens)++;
    }
    token[i] = '\0';

    return token;
}

/**
 * Add a symbolic label for the specified address.
 */
void addLabel(const char* label, uint16_t address)
{
    assert(label);
    labels[label] = address;
}

#include <map>
#include <string>

/*
 * Return the address of the given label.
 */
uint16_t findLabel(const char* label)
{
    assert(label);

    SymbolAddressMap::iterator it = labels.find(std::string(label));

    return (it != labels.end()) ? it->second : 0;
}			

/*
 * Returns the offset between two addresses.
 */
uint8_t calcOffset(uint16_t from, uint16_t to)
{
    short delta = to - from;

    if (delta > 127 || delta < -128)
    {
        printf("Error: Offset from $%04x to $%04x out of range.", from, to);
        exit(-5); // @fixme shouldn't really exit
    }

    return (uint8_t)delta;
}

/**
 * Adds an unresolved branch label to the table for resolution after the
 * entire program has been assembled and all label addresses are know.
 */
void addBranch(const char* branch, uint16_t address)
{
    assert(branch);
    branches[address] = branch;
}

/**
 * Initializes the instruction table and corresponding
 * functions, data structures, etc.
 */
int initialize()
{
    int err = ticker_init(1); // @todo make speed a config option
    if (err != 0)
    {
        printf("Warning: clock timing initialization failed, error %d", err);
    }
    MAP_INITIALIZE;
    MAP_INSTRUCTION(ADCA);
    MAP_INSTRUCTION(ADCI);
    MAP_INSTRUCTION(ADCIX);
    MAP_INSTRUCTION(ADCIY);
    MAP_INSTRUCTION(ADCX);
    MAP_INSTRUCTION(ADCY);
    MAP_INSTRUCTION(ADCZ);
    MAP_INSTRUCTION(ADCZX);
    MAP_INSTRUCTION(ANDA);
    MAP_INSTRUCTION(ANDI);
    MAP_INSTRUCTION(ANDIX);
    MAP_INSTRUCTION(ANDIY);
    MAP_INSTRUCTION(ANDX);
    MAP_INSTRUCTION(ANDY);
    MAP_INSTRUCTION(ANDZ);
    MAP_INSTRUCTION(ANDZX);
    MAP_INSTRUCTION(ASL);
    MAP_INSTRUCTION(ASLA);
    MAP_INSTRUCTION(ASLX);
    MAP_INSTRUCTION(ASLZ);
    MAP_INSTRUCTION(ASLZX);
    MAP_INSTRUCTION(BCC);
    MAP_INSTRUCTION(BCS);
    MAP_INSTRUCTION(BEQ);
    MAP_INSTRUCTION(BIT);
    MAP_INSTRUCTION(BITZ);
    MAP_INSTRUCTION(BMI);
    MAP_INSTRUCTION(BNE);
    MAP_INSTRUCTION(BPL);
    MAP_INSTRUCTION(BRK);
    MAP_INSTRUCTION(BVC);
    MAP_INSTRUCTION(BVS);
    MAP_INSTRUCTION(CLC);
    MAP_INSTRUCTION(CLD);
    MAP_INSTRUCTION(CLI);
    MAP_INSTRUCTION(CLV);
    MAP_INSTRUCTION(CMPA);
    MAP_INSTRUCTION(CMPI);
    MAP_INSTRUCTION(CMPIX);
    MAP_INSTRUCTION(CMPIY);
    MAP_INSTRUCTION(CMPX);
    MAP_INSTRUCTION(CMPY);
    MAP_INSTRUCTION(CMPZ);
    MAP_INSTRUCTION(CMPZX);
    MAP_INSTRUCTION(CPXA);
    MAP_INSTRUCTION(CPXI);
    MAP_INSTRUCTION(CPXZ);
    MAP_INSTRUCTION(CPYA);
    MAP_INSTRUCTION(CPYI);
    MAP_INSTRUCTION(CPYZ);
    MAP_INSTRUCTION(DECA);
    MAP_INSTRUCTION(DECX);
    MAP_INSTRUCTION(DECZ);
    MAP_INSTRUCTION(DECZX);
    MAP_INSTRUCTION(DEX);
    MAP_INSTRUCTION(DEY);
    MAP_INSTRUCTION(EORI);
    MAP_INSTRUCTION(EORA);
    MAP_INSTRUCTION(EORIX);
    MAP_INSTRUCTION(EORIY);
    MAP_INSTRUCTION(EORX);
    MAP_INSTRUCTION(EORY);
    MAP_INSTRUCTION(EORZ);
    MAP_INSTRUCTION(EORZX);
    MAP_INSTRUCTION(INCA);
    MAP_INSTRUCTION(INCX);
    MAP_INSTRUCTION(INCZ);
    MAP_INSTRUCTION(INCZX);
    MAP_INSTRUCTION(INX);
    MAP_INSTRUCTION(INY);
    MAP_INSTRUCTION(JMP);
    MAP_INSTRUCTION(JMPI);
    MAP_INSTRUCTION(JSR);
    MAP_INSTRUCTION(LDAA);
    MAP_INSTRUCTION(LDAI);
    MAP_INSTRUCTION(LDAIX);
    MAP_INSTRUCTION(LDAIY);
    MAP_INSTRUCTION(LDAX);
    MAP_INSTRUCTION(LDAY);
    MAP_INSTRUCTION(LDAZ);
    MAP_INSTRUCTION(LDAZX);
    MAP_INSTRUCTION(LDXA);
    MAP_INSTRUCTION(LDXY);
    MAP_INSTRUCTION(LDXI);
    MAP_INSTRUCTION(LDXZ);
    MAP_INSTRUCTION(LDXZY);
    MAP_INSTRUCTION(LDYA);
    MAP_INSTRUCTION(LDYI);
    MAP_INSTRUCTION(LDYZ);
    MAP_INSTRUCTION(LDYX);
    MAP_INSTRUCTION(LDYZX);
    MAP_INSTRUCTION(LSR);
    MAP_INSTRUCTION(LSRA);
    MAP_INSTRUCTION(LSRX);
    MAP_INSTRUCTION(LSRZ);
    MAP_INSTRUCTION(LSRZX);
    MAP_INSTRUCTION(NOP);
    MAP_INSTRUCTION(ORAI);
    MAP_INSTRUCTION(ORAA);
    MAP_INSTRUCTION(ORAIX);
    MAP_INSTRUCTION(ORAIY);
    MAP_INSTRUCTION(ORAX);
    MAP_INSTRUCTION(ORAY);
    MAP_INSTRUCTION(ORAZ);
    MAP_INSTRUCTION(ORAZX);
    MAP_INSTRUCTION(PHA);
    MAP_INSTRUCTION(PHP);
    MAP_INSTRUCTION(PLA);
    MAP_INSTRUCTION(PLP);
    MAP_INSTRUCTION(ROL);
    MAP_INSTRUCTION(ROLA);
    MAP_INSTRUCTION(ROLX);
    MAP_INSTRUCTION(ROLZ);
    MAP_INSTRUCTION(ROLZX);
    MAP_INSTRUCTION(ROR);
    MAP_INSTRUCTION(RORA);
    MAP_INSTRUCTION(RORX);
    MAP_INSTRUCTION(RORZ);
    MAP_INSTRUCTION(RORZX);
    MAP_INSTRUCTION(RTI);
    MAP_INSTRUCTION(RTS);
    MAP_INSTRUCTION(SBCA);
    MAP_INSTRUCTION(SBCI);
    MAP_INSTRUCTION(SBCIX);
    MAP_INSTRUCTION(SBCIY);
    MAP_INSTRUCTION(SBCX);
    MAP_INSTRUCTION(SBCY);
    MAP_INSTRUCTION(SBCZ);
    MAP_INSTRUCTION(SBCZX);
    MAP_INSTRUCTION(SEC);
    MAP_INSTRUCTION(SED);
    MAP_INSTRUCTION(SEI);
    MAP_INSTRUCTION(STAA);
    MAP_INSTRUCTION(STAIX);
    MAP_INSTRUCTION(STAIY);
    MAP_INSTRUCTION(STAX);
    MAP_INSTRUCTION(STAY);
    MAP_INSTRUCTION(STAZ);
    MAP_INSTRUCTION(STAZX);
    MAP_INSTRUCTION(STXA);
    MAP_INSTRUCTION(STXZ);
    MAP_INSTRUCTION(STXZY);
    MAP_INSTRUCTION(STYA);
    MAP_INSTRUCTION(STYZ);
    MAP_INSTRUCTION(STYZX);
    MAP_INSTRUCTION(TAX);
    MAP_INSTRUCTION(TAY);
    MAP_INSTRUCTION(TSX);
    MAP_INSTRUCTION(TXA);
    MAP_INSTRUCTION(TXS);
    MAP_INSTRUCTION(TYA);
    bInitialized = true;
    return 0;
}

/**
 * Cleanup of data structures before program exit.
 */
int cleanup()
{
    ticker_cleanup(); // @todo emit errors
    bInitialized = false;
    return 0;
}

/**
 * Load an object file from the specified file.
 */
int load(const char* filename)
{
    assert(filename);

    if (bInitialized == false) return -1;

    FILE* fp = fopen(filename, "rb");

    if (fp == NULL) return errno;
    if (k64K != fread(memory, sizeof(char), k64K, fp)) return errno;
    if (0 != fclose(fp)) return errno;

    return 0;
}

/**
 * Save a program to the named file.
 */
int save(const char* filename)
{
    assert(filename);

    if (bInitialized == false) return -1;

    FILE* fp = fopen(filename, "wb");

    if (fp == NULL) return errno;
    if (k64K != fwrite(memory, sizeof(char), k64K, fp)) return errno;
    if (0 != fclose(fp)) return errno;

    return 0;
}

/**
 * Re-initialize data structures prior to assembly.
 */
void prepare()
{
    memset(memory, 0, k64K);
    labels.clear();
    branches.clear();
    breakpoints.clear();
}

/**
 * Find an instruction in the lookup table, returning the index
 * it was found at or -1 if not found.
 */
short lookup(const char* str)
{
    short index = -1;

    assert(str);

    // @todo switch to stl map

    for (int i=0; i < kInstrSetTableSize; i++)
    {
        if (strcmp(str, i6502[i].symbol) == 0)
        {
            index = i;
            break;
        }
    }

    return index;
}

/**
 * Resolve all branches/jumps as part of the assembly process.
 */
int resolve()
{
    FTRACE("Resolving %d branches\n", __FILE__, __LINE__, branches.size());

    // 
    // Resolve branches/jumps
    //
    for (AddressSymbolMap::iterator it=branches.begin(); 
        it != branches.end(); 
        it++)
    {
        uint16_t brAddress = it->first; // address of unresolved branch
        const std::string& brLabel = it->second; // label to branch to

        FTRACE("Resolving branch %s at %04x\n", __FILE__, __LINE__,
            brLabel.c_str(), brAddress);

        //
        // For each branch, get the address for the destination label
        //
        uint16_t address = findLabel(brLabel.c_str());

        FTRACE("Resolved label %s to %04x\n", __FILE__, __LINE__,
            brLabel.c_str(), address);

        if (address)
        {
            //
            // Look for JSR or JMP instructions
            //
            if (i6502[memory[brAddress-1]].symbol[0] == 'J')
            {
                //
                // Store the destination address after the JMP/JSR
                //
                memory[brAddress] = LOBYTE(address);
                memory[brAddress+1] = HIBYTE(address);
            }
            else // branch
            {
                //
                // Branches are relative, so caculate the offset and store
                //
                memory[brAddress] = calcOffset(brAddress+1, address);
            }
        }
        else
        {
            printf("Unresolved branch to label %s\n", brLabel.c_str());
            return -1;
        }
    }
    return 0;
}

/*
 * Load the assembly program from the named file and attempt to
 * assemble it.
 */
int assemble(const char* filename)
{
    assert(filename);

    // @todo add trace statements
    // @todo this whole block and related functions need to be refactored
    // @todo bestow award for worlds longest function

    if (bInitialized == false) return -1;

    FILE* fp = fopen(filename, "r");

    if (fp == NULL) return errno; // @todo correct insufficient info passed to caller

    prepare();

    char line[kMaxLineLength+1];
    int  lineno = 0;
    int  done = 0;
    uint16_t ip = 0;

    while (!done)
    {
        if (fgets(line, kMaxLineLength,fp) != NULL)
        {
            unsigned int tokeno = 0;
            char* tokens = line;
            char  token[kMaxLineLength+1];
            bool  skip = false;

            lineno++;

            uppercase(line);

            FTRACE("Assembler read line: %s",
                __FILE__, __LINE__, line);
            
            // !!! refactor the following assembler to functions

            while (strlen(getToken(token,&tokens)) && skip == false)
            {
                tokeno++;

                FTRACE("Assembler got token (#%02x): %s",
                    __FILE__, __LINE__, tokeno, token);

                if (token[0] == ';') // comment
                {
                    skip = true;
                    FTRACE("Ignoring comment line: %s",
                        __FILE__, __LINE__, line);
                }
                else if (token[0] == '$') // address 
                {
                    if (tokeno == 1) // @todo fix lame state machine
                    {
                        if (strlen(token+1) > 4)
                        {
                            printf("Line %d: wrong number of digits in hex value, ->%s<-\n",
                                lineno, token);
                            return -1; // @todo change to constant (or actually would prefer exceptions everywhere)
                        }
                        else
                        {
                            ip = getHex(token+1);
                        }
                    }
                    else
                    {
                        if (strlen(token+1) > 4)
                        {
                            printf("Line %d: wrong number of digits in hex value, ->%s<-\n",
                                lineno, token);
                            return -2; // @todo change to error value
                        }
                        else
                        {
                            uint16_t hex = getHex(token+1);
                            memory[ip++] = LOBYTE(hex);
                            if (hex > 0xff) memory[ip++] = HIBYTE(hex);

                            FTRACE("Assembler stored address: %04x",
                                __FILE__, __LINE__, hex);
                        }
                    }
                }
                else if (token[0] == '#') // value 
                {
                    if (token[1] == '$') // hex value
                    {
                        if (strlen(token+2) > 2)
                        {
                            printf("Line %d: wrong number of digits in hex value, ->%s<-\n",
                                lineno, token);
                            return -3; // @todo change to error value
                        }
                        else
                        {
                            uint16_t hex = getHex(token+2);
                            memory[ip++] = LOBYTE(hex);

                            FTRACE("Assembler stored value: %02x",
                                __FILE__, __LINE__, hex);
                        }
                    }
                    else // decimal value
                    {
                        if (strlen(token+1) > 3)
                        {
                            printf("Line %d: wrong number of digits in decimal value, ->%s<-\n",
                                lineno, token);
                            return -4; // @todo change to error value
                        }
                        else
                        {
                            char* next = token+1;
                            uint16_t value = (uint16_t)strtol(token+1, &next, 10);

                            if (next == token+1)
                            {
                                printf("Line %d: unexpected decimal value, ->%s<-\n",
                                    lineno, token);
                                return -5; // @todo change to error value
                            }
                            else if (errno == ERANGE)
                            {
                                printf("Line %d: decimal value out of range, ->%s<-\n",
                                    lineno, token);
                                return -6; // @todo change to error value
                            }
                            else
                            {
                                memory[ip++] = LOBYTE(value);

                                FTRACE("Assembler stored value: %02x (%03d)",
                                    __FILE__, __LINE__, value, value);
                            }
                        }
                    }
                }
                else if (strcmp(token, ".DATA") == 0)
                {
                    FTRACE("Assembler processing data section",
                        __FILE__, __LINE__);

                    while (strlen(getToken(token,&tokens)))
                    {
                        uint16_t hex = getHex(token);
                        memory[ip++] = LOBYTE(hex);
                        if (hex > 0xff) memory[ip++] = HIBYTE(hex);

                        FTRACE("Assembler stored data section value: %04x",
                            __FILE__, __LINE__, hex);
                    }
                }
                else if (strncmp(token, line, strlen(token)) == 0)
                {
                    addLabel(token, ip);
                    FTRACE("Assembler recording label: %s at %04x",
                        __FILE__, __LINE__, token, ip);
                }
                else
                {
                    short instruction = lookup(token);

                    FTRACE("Assembler looking up token: %s resolves to opcode %02x",
                        __FILE__, __LINE__, token,instruction);

                    if (instruction >= 0)
                    {
                        FTRACE("Assembler storing instruction: %02x at %04x",
                            __FILE__, __LINE__, instruction,ip);
                        memory[ip++] = (uint8_t)instruction;
                    }
                    else 
                    {
                        // 
                        // Assuming here that the token is a label for a
                        // branch or jump. 
                        //

                        FTRACE("Assembler adding branch to: %s at %04x",
                            __FILE__, __LINE__, token, ip);

                        //
                        // @todo fix this assumption to validate or error out
                        //
                        addBranch(token, ip);

                        //
                        // hack to distinguish between absolute and relative destinations
                        //
                        ip += (i6502[memory[ip-1]].symbol[0] == 'J') ? 2:1;

                        //printf("Line %d: unrecognized instruction, ->%s<-", __FILE__, __LINE__, lineno, token);
                        //exit(-3);
                    }
                }
            }
        }
        else if (feof(fp) != 0)
        {
            done = 1;
            if (0 != fclose(fp)) return errno;
        }
        else
        {
            return errno;
        }
    };

    //
    // Resolve all jumps and branches to their destination addresses or offets
    //
    resolve();

    return 0;
}

/*
 * Reset run-time registers and status bits to defaults.
 */
void reset(uint16_t address)
{
    BP = memory;
    PC = address;
    
    SP = 255;
    CARRY = 0;
    ZERO = 0;
    INTERRUPT = 0;
    DECIMAL = 0;
    BREAK = 0;
    OVERFLOW = 0;
    SIGN = 0;

    A = 0;
    X = 0;
    Y = 0;
    P = 0;
}

/*
 * Print register values to stderr.
 */
void dumpRegisters()
{
    fprintf(stderr, "PC=%04x SP=%02x A=%02x X=%02x Y=%02x P=%02x\n",
        PC, (int)SP, (int)A, (int)X, ( int)Y, (int)P);
}

/*
 * Print flags to stderr.
 */
void dumpFlags()
{
   fprintf(stderr, "S=%01x V=%01x B=%01x D=%01x I=%01x Z=%01x C=%01x\n",
       (int)SIGN, (int)OVERFLOW, (int)BREAK, (int)DECIMAL,
       (int)INTERRUPT, (int)ZERO, (int)CARRY);
}

/*
 * Print stack values to stderr.
 */
void dumpStack()
{
    fprintf(stderr, "Stack Dump...");

    for (uint8_t i=kStackSize-1; i > SP; i--)
    {
        fprintf(stderr, "%02x ", (uint8_t)STACK[i]);
    }

    fprintf(stderr, "\n");
}

/*
 * Print non-zero memory values to stderr.
 */
void dumpMemory(uint16_t first, uint16_t last)
{
    first = (first / 8) * 8;

    fprintf(stderr, "%04x ", first);

    for (uint32_t dump=first; dump <= last; dump++)
    {
        fprintf(stderr, "%02x ", (uint8_t)*(BP+dump));

        if ((dump+1) < last && ((dump+1) % 8) == 0) 
        {
            fprintf(stderr, "\n%04x ", dump+1);
        }
    }

    fprintf(stderr, "\n");
}

/*
 * Print selected items to stderr.
 */
void dump(bool bRegisters, bool bFlags, bool bStack, bool bMemory)
{
    if (bRegisters) dumpRegisters();
    if (bFlags) dumpFlags();
    if (bStack) dumpStack();
    if (bMemory) dumpMemory();
}

/*
 * Turn object code into instruction descriptions (disassembly).
 */
void decodeAt(uint16_t address)
{
    fprintf(stdout, "PC=%04x %s ",
        address, i6502[*(BP+address)].symbol);

    for (int i=1; i <= i6502[*(BP+address)].bytes-1; i++)
    {
        fprintf(stdout, "%02x ", (uint8_t)*(BP+address+i));
    }

    fprintf(stdout, "\n");
}

/**
 * Return the value of memory at the given address.
 */
uint8_t inspect(uint16_t address)
{
    return memory[address];
}

/**
 * Decode object code to symbolic instructions.
 */
void list(uint16_t first, uint16_t last)
{
    for (uint16_t addr=first; addr <= last; addr += i6502[*(BP+addr)].bytes)
    {
        decodeAt(addr);
    }
}

/**
 * Interpret and execute a single instruction.
 */
int step()
{
    FTRACE("PC=%04x OPCODE=%02x (%s) SP=%02x A=%02x X=%02x Y=%02x P=%02x",
        __FILE__, __LINE__,
        PC, (int)*(BP+PC), i6502[*(BP+PC)].symbol, 
        (int)SP, (int)A, (int)X, (int)Y, (int)P);
    FTRACE("S=%01x V=%01x B=%01x D=%01x I=%01x Z=%01x C=%01x",
        __FILE__, __LINE__,
        (int)SIGN, (int)OVERFLOW, (int)BREAK, (int)DECIMAL,
        (int)INTERRUPT, (int)ZERO, (int)CARRY);
 
    assert(i6502[*(BP+PC)].pFunc);

    uint8_t opcode = *(BP+PC);
    i6502[opcode].pFunc();
    ticker_wait(i6502[opcode].cycles);

    return 0;
}

/**
 * Run the object code found at the given address.
 */
int run(uint16_t address)
{
    if (bInitialized == false) return -1;
   
    reset(address);

    for(;BREAK != 1;)
    {
        step();
    }

    return 0;
}

/**
 * Tokenize assembler input.
 */
void tokenize(char* first, char* second, char* third, const char* input)
{
    unsigned int tokeno = 0;
    char* tokens = strdup(input);
    char* parsed = tokens;
    char  token[kMaxLineLength+1];

    assert(first);
    assert(second);
    assert(third);
    assert(input);

    uppercase(tokens);

    *first = '\0';
    *second = '\0';
    *third = '\0';

    while (strlen(getToken(token, &parsed)))
    {
        FTRACE("Tokenize got token (#%d): %s", __FILE__, __LINE__, tokeno, token);

        switch(tokeno++)
        {
            case 0:
                strcpy(first, token);
                break;
            case 1:
                strcpy(second, token);
                break;
            case 2:
                strcpy(third, token);
                break;
            default:
                assert(0);
                break;
        }
    }

    free(tokens);
}

/**
 * Set a breakpoint at the specified address.
 */
void setBreak(uint16_t address)
{
    breakpoints[address] = true;
}

/**
 * Remove a breakpoint.
 */
void clearBreak(uint16_t address)
{
    breakpoints.erase(address);
}

/**
 * List all active breakpoints.
 */
void listBreak()
{
    for (BreakpointMap::iterator it=breakpoints.begin();
        it != breakpoints.end();
        it++)
    {
        printf("%04x\n", it->first);
    }
}

/**
 * Check the given address against active breakpoints.
 */
bool checkBreak(uint16_t address)
{
    if (breakpoints.find(address) != breakpoints.end())
    {
        return true;
    }

    return false;
}

/**
 * Find the command action corresponding to the input command.
 */
ACTION getCommand(const char* command)
{
    assert(command);

    for (int i=0; i < sizeof commands/sizeof(COMMAND_TO_ACTION); i++)
    {
        if (strcmp(command, commands[i].command) == 0 || 
            strcmp(command, commands[i].abbrev) == 0)
        {
            return commands[i].action;
        }
    }

    return kUnknown;
}

/**
 * Print help.
 */
void printDebugHelp()
{
    fprintf(stdout, "Valid commands:\n");
    fprintf(stdout, "\trun (or r)\n");
    fprintf(stdout, "\tstep (or s)\n");
    fprintf(stdout, "\tgo (or g)\n");
    fprintf(stdout, "\tprint (or p) <first> <last>\n");
    fprintf(stdout, "\tregisters (or e)\n");
    fprintf(stdout, "\tflags (or f)\n");
    fprintf(stdout, "\tstack (or a)\n");
    fprintf(stdout, "\tbreak (or b)\n");
    fprintf(stdout, "\tclear (or c)\n");
    fprintf(stdout, "\ttrace (or t)\n");
    fprintf(stdout, "\tlist (or l) <first> <last>\n");
    fprintf(stdout, "\texit (or x)\n");
    fprintf(stdout, "\tquit (or q)\n");
    fprintf(stdout, "\thelp (or h)\n");
}

/**
 * Enter the interactive debugger using the object code at given address.
 * @fixme needs to work in envs that aren't CLI-based (i.e. GUI driven)
 */
int debug(uint16_t address)
{
    if (bInitialized == false) return -1;
   
    reset(address);

    char line[kMaxLineLength];
    bool bRead = true;

    for(;BREAK != 1;)
    {
        if (bRead)
        {
            printf("> ");
            fflush(stdout);

            if (fgets(line, kMaxLineLength, stdin) != NULL)
            {
                char command[kMaxLineLength+1];
                char param1[kMaxLineLength+1];
                char param2[kMaxLineLength+1];

                FTRACE("Debugger read line: %s", __FILE__ , __LINE__, line);
                
                tokenize(command, param1, param2, line);

                FTRACE("Debugger command and params are: CMD=%s, PARAM1=%s, PARAM2=%s",
                    __FILE__, __LINE__, command,param1,param2);

                switch (getCommand(command))
                {
                case kUnknown:
                    fprintf(stderr, "Unknown command: %s\n", command);
                    break;
                case kExit:
                    return 0;
                    break;
                case kPrint:
                    {
                        short addr1 = strlen(param1) ? getHex(param1):PC;
                        short addr2 = strlen(param2) ? getHex(param2):addr1;
                        dumpMemory(addr1, addr2);
                    }
                    break;
                case kStack:
                    dumpStack();
                    break;
                case kRegisters:
                    dumpRegisters();
                    break;
                case kFlags:
                    dumpFlags();
                    break;
                case kStep:
                    step();
                    break;
                case kContinue:
                    bRead = false;
                    break;
                case kBreak:
                    if (strlen(param1)) setBreak(getHex(param1));
                    else listBreak();
                    break;
                case kClear:
                    clearBreak(getHex(param1));
                    break;
                case kRun:
                    {
                        short addr = strlen(param1) ? getHex(param1):address;
                        reset(addr);
                        bRead = false;
                    }
                    break;
                case kTrace:
                    g_bTrace = !g_bTrace;
                    break;
                case kList:
                    {
                        short addr1 = strlen(param1) ? getHex(param1):PC;
                        short addr2 = strlen(param2) ? getHex(param2):addr1;
                        list(addr1,addr2);
                    }
                    break;
                case kAssert:
                    {
                        uint16_t addr = getHex(param1);
                        uint8_t val = (uint8_t)getHex(param2);
                        fprintf(stderr, "%s\n", assertmem(addr,val) ? "true":"false");
                    }
                    break;
                case kHelp:
                    printDebugHelp();
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            step();
            if (checkBreak(PC)) bRead = true;
        }
    }

    return 0;
}

/**
 * Print version string and build timestamp
 */
void printVersion()
{
    fprintf(stderr, "%s (Build Time: %s)\n", kVersion, __TIMESTAMP__);
}

/*
 * Print the instruction table to stderr.
 */
void printInstructions()
{
    for (unsigned int ii=0; ii < kInstrSetTableSize; ii++)
    {
        if (i6502[ii].symbol) fprintf(stderr, "%s - %s\n", i6502[ii].symbol, i6502[ii].desc);
    }
}

