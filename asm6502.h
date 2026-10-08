#ifndef _ASM6502_H_
#define _ASM6502_H_

#include "platform.h"

/**
 * Two-pass assembler for the 6502 emulator. assemble() in l6502.h is the
 * public entry point; it prepares the emulator and calls asmAssemble().
 * See docs/ASSEMBLER_PLAN.md for the design and the syntax accepted.
 */

/**
 * Assemble the named source file into the 64K image at memory. Every error
 * is reported to stderr as "file:line:col: error: message" and assembly
 * continues so that all errors in a file are reported together.
 *
 * @return the number of errors (0 on success), or -1 if the file could not
 *         be opened (errno is set).
 */
int asmAssemble(const char* filename, uint8_t* memory);

/*
 * Instruction table access the assembler needs from the emulator. These are
 * implemented in l6502.cpp, which owns the table.
 */

/**
 * @return the opcode for an instruction mnemonic (e.g. "LDAI"), or -1.
 */
int asmLookupInstruction(const char* symbol);

/**
 * @return the total length in bytes of the instruction with this opcode,
 *         including the opcode byte; 0 for an unimplemented opcode.
 */
uint8_t asmInstructionBytes(uint8_t opcode);

#endif
