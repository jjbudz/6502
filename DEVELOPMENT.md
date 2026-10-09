# Development Guide

This guide provides comprehensive information for developers working on the 6502 emulator project.

## Table of Contents
1. [Project Structure](#project-structure)
2. [Assembler Guide](#assembler-guide)
3. [Emulator Implementation Guide](#emulator-implementation-guide)
4. [Testing Guide](#testing-guide)

---

## Project Structure

### Overview
This project is a 6502 CPU emulator written in C++ that includes:
- A 6502 CPU emulator core
- A basic assembler with parser
- A debugger with breakpoints
- An interactive command-line interface
- Unit tests for validation

### Directory Layout
```
6502/
├── bin/                  # Binary outputs (by TYPE/PLATFORM)
│   ├── debug/
│   └── release/
├── lib/                  # Library outputs (by TYPE/PLATFORM)
│   ├── debug/
│   └── release/
├── obj/                  # Object files (by TYPE/PLATFORM)
│   ├── debug/
│   └── release/
├── test/                 # Unit tests and test infrastructure
│   ├── *.asm            # Test assembly files
│   ├── makefile         # Test runner makefile
│   ├── unittest.script  # Legacy test script
│   └── README.md        # Test documentation
├── *.cpp, *.h           # Source and header files
├── makefile             # Main build file
├── include.mk           # Build system configuration
├── README.md            # Project overview
├── BUILDING.md          # Build instructions
├── CHEATSHEET.md        # Usage reference
└── TODO.md              # Known issues and future work
```

### Core Source Files

#### Main Components
- **`main.cpp`**: Command-line interface and entry point
  - Argument parsing
  - Mode selection (compile, load, run, debug)
  - Main execution loop

- **`l6502.cpp` / `l6502.h`**: Core 6502 emulator library
  - CPU state (registers: A, X, Y, PC, SP, P)
  - Memory (64KB addressable space)
  - Instruction set implementation (~150+ opcodes)
  - Debugger
  - Execution engine

- **`asm6502.cpp` / `asm6502.h`**: Two-pass assembler
  - Lexer and line parser (`lexLine`, `parseLine`)
  - Pass 1 assigns addresses and records labels; pass 2 emits bytes and
    resolves label operands
  - Reports every error as `file:line:col: error: message` and continues
  - `assemble()` in `l6502.cpp` is the entry point; it clears memory and
    calls `asmAssemble()`
  - See `docs/ASSEMBLER_PLAN.md` for the rework this is part of

- **`ftrace.cpp` / `ftrace.h`**: Function tracing/debugging utility
  - Conditional tracing via FTRACE environment variable
  - Debug output to file
  - Enable with: `export FTRACE=1`

- **`ticker.cpp` / `ticker.h`**: CPU timing/clock simulation
  - Simulates CPU clock cycles
  - Configurable clock rate (default 1 MHz)
  - Used for timing-accurate emulation

- **`util.cpp` / `util.h`**: Utility functions
  - String manipulation
  - Hex conversion
  - Case conversion

- **`platform.h`**: Platform-specific definitions
  - Cross-platform compatibility
  - Type definitions

### Build System

The project uses a GNU Make-based build system with platform and type detection:

#### Build Variables
- **`TYPE`**: Build type (`debug`, `release`, `profile`)
- **`PLATFORM`**: Target platform (`linux`, `macos`)
  - Auto-detected if not specified
- **`CCFLAGS`**: Compiler flags (set based on TYPE and PLATFORM)

#### Key Makefiles
- **`makefile`**: Main build configuration
  - Defines library sources
  - Defines binary targets
  - Includes `include.mk`
  
- **`include.mk`**: Build system core
  - Platform detection
  - Compiler flag configuration
  - Standard build targets
  - Directory structure creation

- **`test/makefile`**: Test runner
  - Individual test targets
  - Aggregate test execution
  - Test result validation

#### Build Targets
```bash
make                     # Build everything (default: debug)
make TYPE=release        # Build release version
make TYPE=debug          # Build debug version
make clean               # Remove build artifacts
make test                # Run all tests
```

### Memory Organization

The emulator provides a 64KB address space (16-bit addressing):

```
$0000 - $00FF    Zero Page (256 bytes)
$0100 - $01FF    Stack (256 bytes, grows downward from $01FF)
$0200 - $3FFF    General RAM
$4000 - $FFFF    General RAM / Program space

Common conventions in tests:
$4000            Default program start address
$8000+           Test result storage area
```

### CPU State

The emulator maintains the following 6502 CPU state:

#### Registers
- **A**: Accumulator (8-bit)
- **X**: Index Register X (8-bit)
- **Y**: Index Register Y (8-bit)
- **PC**: Program Counter (16-bit)
- **SP**: Stack Pointer (8-bit, points into $0100-$01FF)
- **P**: Processor Status (8-bit flags)

#### Status Flags (P register)
- **Bit 0 (C)**: Carry flag
- **Bit 1 (Z)**: Zero flag
- **Bit 2 (I)**: Interrupt disable flag
- **Bit 3 (D)**: Decimal mode flag
- **Bit 4 (B)**: Break flag
- **Bit 5**: Unused (always 1)
- **Bit 6 (V)**: Overflow flag
- **Bit 7 (N)**: Negative/Sign flag

---

## Assembler Guide

### Overview
The project includes a two-pass assembler (`asm6502.cpp`) that converts assembly source files (`.asm`) into a 64K memory image. It accepts standard 6502 syntax and, for compatibility with the existing test programs, a legacy syntax in which the addressing mode is spelled in the mnemonic. The two may be mixed in one file.

### Syntax Rules

A line is:

```
[$addr] [label[:]] MNEMONIC [operand]   ; comment
[$addr] [label[:]] .DATA value ...      ; comment
```

Source is case-insensitive. A leading `$addr` sets the address at which the following code is placed.

#### Numbers
Hexadecimal values are prefixed with `$`, binary with `%`, decimal values are bare digits, and a character in single quotes is its ASCII code:
```asm
STA $8000     ; hex address
LDA #59       ; decimal immediate
LDA #$5A      ; hex immediate
AND #%00001111
LDA #'A'      ; $41
```

#### Expressions
Wherever a value is expected, an expression may be written instead. Expressions combine numbers, labels, constants and `*` (the address of the current instruction) with `+` and `-`, and the unary operators `<` (low byte), `>` (high byte) and `-` (negate). Arithmetic is 16-bit.
```asm
LDA TABLE+1       ; the byte after TABLE
LDA #<TARGET      ; low byte of TARGET's address
LDA #>TARGET      ;  high byte
JMP *+6           ; six bytes past this instruction
LDA #-1           ; an immediate may be a negative byte: $FF
LDA #10-3
```
Labels used in an expression may be defined later in the file. The exceptions are `.ORG` and constant definitions, which must be computable where they appear.

#### Constants
`NAME = expression` (or `NAME .EQU expression`) defines a constant; it can be used anywhere a label can, and a constant under `$100` selects zero page like a label would. `* = expression` is the same as `.ORG`.
```asm
BASE = $50
MASK = %00001111
* = $4000
        LDA BASE+2    ; zero page $52
```

#### Directives
| Directive | Effect |
|-----------|--------|
| `.ORG e` | Set the address for the code that follows (same as a leading `$addr` or `* = e`) |
| `NAME .EQU e` | Define a constant (same as `NAME = e`) |
| `.BYTE e, e, ...` | One byte per expression; a `"string"` gives one byte per character, case preserved |
| `.WORD e, e, ...` | Two bytes per expression, low byte first |
| `.TEXT "s", e, ...` | The same as `.BYTE`, for readability: `.TEXT "Hi", 0` is a terminated string |
| `.DATA h h ...` | Legacy: space-separated hex values, with or without `$`, one byte each or two (low first) above `$FF` |

```asm
TABLE   .BYTE 1, 2, 3
PTR     .WORD $1234, TABLE
MSG     .TEXT "Hello", 0
```

#### Labels
A label is an identifier in the first column of a line, or any identifier followed by `:` (which may be indented). Labels can be used wherever a value is expected, including as jump and branch targets, and may be defined after the instruction that refers to them. Defining a label twice is an error.

```asm
foo   JMP bar       ; Jump to label 'bar'
      LDA #00
      STA $8000
      BRK
bar:  LDA #01       ; Label 'bar' definition
      STA $8000
      BRK
```

#### Addressing Modes: Standard Syntax
With a bare mnemonic, the operand's shape selects the addressing mode, as in any 6502 assembler:

| Mode | Example | Bytes |
|------|---------|-------|
| Implied / Accumulator | `INX`, `LSR A` | 1 |
| Immediate | `LDA #$42` | 2 |
| Zero page | `LDA $80` | 2 |
| Zero page,X / ,Y | `LDA $80,X`, `LDX $80,Y` | 2 |
| Absolute | `LDA $8000` | 3 |
| Absolute,X / ,Y | `LDA $8000,X`, `LDA $8000,Y` | 3 |
| (Indirect) | `JMP ($1234)` | 3 |
| (Indirect,X) | `LDA ($40,X)` | 2 |
| (Indirect),Y | `LDA ($40),Y` | 2 |
| Relative | `BNE loop`, `BEQ $4010` | 2 |

Zero page is chosen over absolute when the instruction has a zero page form and the value is known to fit: a literal under `$100` written with fewer than four hex digits (`$80` is zero page, `$0080` is absolute), or a label already defined at an address under `$100`. A label defined later in the file is assumed absolute. Branch targets may be labels or literal addresses and must lie within -128..+127 bytes of the next instruction.

Using a mode the instruction does not have (`LDX $10,X`) is an error that names the instruction and the mode.

#### Addressing Modes: Legacy Syntax
Each opcode also has a suffixed name that fixes its addressing mode (`6502 -i` lists them):

| Suffix | Mode | Example |
|--------|------|---------|
| `I` | Immediate | `LDAI #$42` |
| `Z`, `ZX`, `ZY` | Zero page, zero page,X, zero page,Y | `LDAZ $80`, `LDAZX $80`, `LDXZY $80` |
| `A`, `X`, `Y` | Absolute, absolute,X, absolute,Y | `LDAA $8000`, `LDAX $8000`, `LDAY $8000` |
| `IX`, `IY` | (Indirect,X), (indirect),Y | `LDAIX $40`, `LDAIY $40` |
| (none) | Implied, relative, or absolute (`JMP`, `JSR`, `BIT`) | `INX`, `BNE loop`, `JMP $4000` |

With a legacy mnemonic the operand is a plain value (`#$hh`, `#ddd`, `$hh`, `$hhhh` or a label) and must be the width the instruction needs: a 3-byte instruction requires a 4-digit address, a 2-byte one a value under `$100` written in fewer digits.

### Common Instructions

#### Load/Store Operations
```asm
LDAI #$42     ; Load accumulator immediate
LDAA $8000    ; Load accumulator from address
LDAZ $80      ; Load accumulator from zero page
STAA $8000    ; Store accumulator to address
STAZ $80      ; Store accumulator to zero page
```

#### Register Transfers
```asm
TAX           ; Transfer A to X
TAY           ; Transfer A to Y
TXA           ; Transfer X to A
TYA           ; Transfer Y to A
TXS           ; Transfer X to stack pointer
TSX           ; Transfer stack pointer to X
```

#### Stack Operations
```asm
PHA           ; Push accumulator to stack
PLA           ; Pull accumulator from stack
PHP           ; Push processor status to stack
PLP           ; Pull processor status from stack
```

#### Arithmetic
```asm
ADCI #$10     ; Add with carry immediate
SBCI #$05     ; Subtract with carry immediate
```

#### Increment/Decrement
```asm
INX           ; Increment X
INY           ; Increment Y
DEX           ; Decrement X
DEY           ; Decrement Y
```

#### Flag Operations
```asm
CLC           ; Clear carry flag
SEC           ; Set carry flag
CLD           ; Clear decimal flag
SED           ; Set decimal flag
CLI           ; Clear interrupt flag
SEI           ; Set interrupt flag
CLV           ; Clear overflow flag
```

#### Control Flow
```asm
JMP label     ; Jump to label
JMP ($0010)   ; Jump to the address stored at $0010/$0011
JSR sub       ; Call subroutine; RTS returns
BNE loop      ; Branch to label (relative, within -128..+127 bytes)
BEQ $4010     ; Branch to a literal address
BRK           ; Break (halt execution)
NOP           ; No operation
```

### Assembly File Format

A typical assembly file structure:
```asm
; Program start address
$4000   LDAI #$35      ; Load value
        
        TAX             ; Transfer to X
        DEX             ; Decrement
        TXA             ; Transfer back
        
        STAZ $40        ; Store result
        BRK             ; End program
```

Key conventions:
- First line often specifies start address (e.g., `$4000`)
- Instructions can be indented for readability
- Comments start with `;` and run to the end of the line
- Programs should end with `BRK`

### Assembler Limitations

Current known limitations (see `docs/ASSEMBLER_PLAN.md` for what is planned):
1. **Hex Format**: Requires `#$XX` for hex immediate values, not `#XX`
2. **Expressions** have only `+`, `-`, `<`, `>` and unary `-`: no multiplication, shifts, bitwise operators or parentheses for grouping
3. **Forward references are absolute**: a label defined later in the file is encoded with a 16-bit operand even if it turns out to be in zero page (write `<LABEL` to force a byte)
4. **No macros, includes, conditional assembly or local labels**

Errors are reported with file, line and column, and all errors in a file are reported in one run. Unknown mnemonics, duplicate labels, and operands of the wrong size for their instruction are errors.

### Using the Assembler

#### Command-Line Options
```bash
# Compile and run
6502 -c program.asm -r 4000

# Compile, save object file, and run
6502 -c program.asm -s program.6502 -r 4000

# Load and run object file
6502 -l program.6502 -r 4000

# Compile and debug
6502 -c program.asm -d 4000

# Compile and print a listing: address, bytes and source for every line
6502 -c program.asm -L

# Run from the reset vector, the way real hardware starts. The program
# sets the vector itself:
#     * = $FFFC
#     .WORD start
6502 -c program.asm -r
```

#### List Available Instructions
```bash
6502 -i
```

This displays all available instruction mnemonics recognized by the assembler.

### Assembly Examples

See the `/test` directory for working examples:
- `LDAI1.asm`, `LDAI2.asm`, `LDAI3.asm`: Load accumulator tests
- `PHA.asm`: Stack operations
- `test05.asm`: Multiple register operations
- `sample.asm`, `sample2.asm`: Sample programs

---

## Emulator Implementation Guide

### Core Architecture

The emulator is built around a fetch-decode-execute cycle that interprets 6502 machine code.

### Key Data Structures

#### Instruction Table
```cpp
struct Instruction {
    const char* symbol;     // Mnemonic (e.g., "LDAI")
    const char* desc;       // Description
    void (*func)(void);     // Execution function pointer
    uint8_t opcode;         // Opcode byte
    uint8_t bytes;          // Instruction size in bytes
    uint8_t cycles;         // Clock cycles required
};
```

The instruction table (`i6502[]`) is a 256-entry array indexed by opcode that maps each possible byte value to its corresponding instruction.

#### CPU State
```cpp
uint8_t A;              // Accumulator
uint8_t X, Y;           // Index registers
uint16_t PC;            // Program counter
uint8_t SP;             // Stack pointer
uint8_t P;              // Processor status
uint8_t memory[64K];    // Memory space
```

### Instruction Implementation Pattern

Each instruction is implemented as a function following this pattern:

```cpp
INSTRUCTION(LDAI, 0xA9, 2, 2, "Load Accumulator Immediate")
{
    A = memory[PC+1];           // Fetch operand
    setZero(A == 0);            // Update Z flag
    setSign(A & 0x80);          // Update N flag
    PC += LDAISZ;               // Advance PC by instruction size
    addCycles(LDAICYC);         // Account for timing
}
```

Key elements:
1. **INSTRUCTION macro**: Defines opcode, size, cycles, and description
2. **Operation logic**: Implements the instruction behavior
3. **Flag updates**: Modifies status flags as appropriate
4. **PC advancement**: Moves to next instruction
5. **Cycle accounting**: Tracks CPU timing

### Adding New Instructions

To implement a new instruction:

1. **Define the instruction** using the INSTRUCTION macro:
```cpp
INSTRUCTION(NEWINST, 0xXX, size, cycles, "Description")
{
    // Implementation
}
```

2. **Map it to the opcode** in the instruction table:
```cpp
MAP(NEWINST);
```

3. **Implement the operation logic**:
   - Read operands from memory if needed
   - Perform the operation
   - Update affected registers
   - Set/clear appropriate flags
   - Advance PC by instruction size
   - Add CPU cycles

4. **Update flag handling** (as appropriate):
```cpp
setZero(result == 0);      // Z flag
setSign(result & 0x80);    // N flag (bit 7)
setCarry(carry_occurred);  // C flag
setOverflow(v_occurred);   // V flag
```

### Execution Flow

#### Main Execution Loop (`run()`)
```cpp
int run(uint16_t address) {
    PC = address;
    while (true) {
        if (checkBreak(PC)) break;  // Breakpoint check
        
        uint8_t opcode = memory[PC];
        Instruction* inst = &i6502[opcode];
        
        if (inst->func == NULL) {
            // Unimplemented instruction
            return ERROR;
        }
        
        inst->func();  // Execute instruction
        
        if (opcode == BRK) break;  // Halt on BRK
        
        ticker_wait_for_cycles();  // Timing simulation
    }
    return SUCCESS;
}
```

#### Single Step (`step()`)
The `step()` function executes one instruction and is used by the debugger.

### Debugger

The debugger provides an interactive environment for examining and controlling program execution.

#### Debugger Commands
- `help` - Show available commands
- `run` - Continue execution
- `step` - Execute one instruction
- `list [start] [end]` - Disassemble memory range
- `break <address>` - Set breakpoint
- `clear <address>` - Clear breakpoint
- `breakpoints` - List all breakpoints
- `registers` - Display register values
- `flags` - Display status flags
- `memory [start] [end]` - Dump memory
- `quit` - Exit debugger

#### Entering the Debugger
```bash
# Debug from address $4000
6502 -c program.asm -d 4000

# With trace enabled
6502 -c program.asm -d 4000 -t
```

### Memory Management

#### Reading Memory
```cpp
uint8_t value = memory[address];
uint8_t value = inspect(address);  // Public API
```

#### Writing Memory
```cpp
memory[address] = value;
```

#### Stack Operations
The stack lives at `$0100-$01FF` and grows downward:
```cpp
// Push
memory[0x0100 + SP] = value;
SP--;

// Pull
SP++;
value = memory[0x0100 + SP];
```

### Timing Simulation

The `ticker` module provides CPU clock simulation:

```cpp
initialize(1000000);  // Initialize with 1 MHz clock

// In instruction:
addCycles(cycles);    // Add instruction cycle count
ticker_wait_for_cycles();  // Wait for real-time equivalent
```

This enables timing-accurate emulation for applications that depend on CPU speed.

### Iteration Workflow

When implementing or modifying the emulator:

1. **Plan the change**: Identify which instructions or features need modification
2. **Update instruction implementations**: Modify the relevant INSTRUCTION definitions
3. **Update instruction table**: Ensure MAP() calls are correct
4. **Write tests**: Create `.asm` test files (see Testing Guide)
5. **Build**: `make TYPE=debug`
6. **Test**: `make test` or run specific tests
7. **Debug**: Use debugger to step through test cases
8. **Validate**: Ensure existing tests still pass
9. **Document**: Update comments and documentation

### Debugging Tips

1. **Enable tracing**: `export FTRACE=1` before running
2. **Use the debugger**: Step through instructions with `-d` flag
3. **Check registers**: Use `-pr` flag to print registers on exit
4. **Dump memory**: Use `-pm` flag to see memory state
5. **Trace execution**: Use `-t` flag for instruction trace
6. **Test incrementally**: Test each instruction individually
7. **Compare with spec**: Reference the official 6502 documentation

### Reference Resources

- [6502 Instruction Set Reference](http://www.6502.org/tutorials/6502opcodes.html)
- [6502 Addressing Modes](http://www.obelisk.demon.co.uk/6502/addressing.html)
- [6502 Programming Manual](http://www.6502.org/documents/)

---

## Testing Guide

### Test Infrastructure

The project uses a makefile-based test system with two complementary approaches:

1. **Makefile targets**: Modern, structured test execution
2. **unittest.script**: Legacy shell script for compatibility
3. **Assembler golden files** (`make golden`): every `.asm` must assemble to
   the exact bytes recorded in `test/golden/`; regenerate with
   `make golden-update` after an intentional change
4. **Assembler negative tests** (`make fail`): every `test/fail/*.asm` must
   fail to assemble

See `test/README.md` for details on the last two.

### Test Organization

Tests are located in the `/test` directory:
```
test/
├── ADCI.asm         # Add with carry immediate
├── LDAI1.asm        # Load accumulator tests
├── LDAI2.asm
├── LDAI3.asm
├── CLC.asm          # Clear carry flag
├── SEC.asm          # Set carry flag
├── NOP.asm          # No operation
├── PHA.asm          # Push accumulator
├── test00.asm       # Complex addressing modes
├── test01.asm       # Complex logical operations
├── test05.asm       # Multiple register ops
├── timing.asm       # Timing validation
├── golden/          # Expected assembler output (xxd dumps), one per .asm
├── fail/            # Sources that must fail to assemble
├── golden.sh        # Golden-file check/update
├── fail.sh          # Negative test runner
├── makefile         # Test runner
└── unittest.script  # Legacy test script
```

### Running Tests

#### Run All Tests
```bash
# From project root
make test

# From test directory
cd test
make test PLATFORM=linux TYPE=debug
```

#### Run Individual Tests
```bash
cd test
make test-LDAI1
make test-PHA
make test-test05
```

#### Run Multiple Specific Tests
```bash
make test-LDAI1 test-LDAI2 test-LDAI3
```

### Test File Structure

A test file should:

1. **Start at a consistent address** (convention: `$4000`)
2. **Perform operations** to test specific functionality
3. **Store result** at a predictable memory location
4. **End with BRK** to halt execution

Example test (`LDAI2.asm`):
```asm
$4000   LDAI #$7f      ; Load value $7f
        STAA $8000     ; Store at test result address
        BRK            ; Halt
```

### Test Assertions

Tests use the `-a` flag to assert expected memory values:

```bash
6502 -c LDAI2.asm -r 4000 -a 8000:7f
```

Format: `-a <address>:<expected_value>`
- `address`: 16-bit hex address (without $)
- `expected_value`: 8-bit hex value (without $)

The emulator will:
- Run the program
- Check the value at the specified address
- Print: `Assert $8000:7f=7f true` (pass) or `Assert $8000:7f=XX false` (fail)
- Return exit code 0 (pass) or 1 (fail)

### Adding New Tests

To add a new test:

#### 1. Create the Test File
```asm
; mynewtest.asm
$4000   LDAI #$42
        TAX
        TXA
        STAA $8000
        BRK
```

#### 2. Add Makefile Target
Edit `test/makefile` and add:
```makefile
test-mynewtest:
	@echo "Test mynewtest"
	$(EMU) -c mynewtest.asm -r 4000 -a 8000:42
```

#### 3. Add to Test Suite
Add your test to the `test` target's dependencies:
```makefile
test:
	@$(MAKE) test-ADCI || true
	@$(MAKE) test-mynewtest || true  # Add this line
	# ... other tests
```

#### 4. Run the Test
```bash
cd test
make test-mynewtest
```

### Test Categories

#### Instruction Tests
Test individual instructions:
- **LDAI1, LDAI2, LDAI3**: Load accumulator immediate
- **PHA**: Push accumulator to stack
- **CLC, SEC**: Flag operations
- **NOP**: No operation

#### Integration Tests
Test multiple instructions together:
- **test05**: Register transfers, stack operations
- **test00**: Complex addressing modes
- **test01**: Logical operations

#### Timing Tests
Test CPU timing simulation:
- **timing**: Validates clock rate accuracy

### Test Result Validation

#### Passing Test
```
Test LDAI2
../bin/debug/linux/6502 -c LDAI2.asm -r 4000 -a 8000:7f
Assert $8000:7f=7f true
```

#### Failing Test
```
Test test00
../bin/debug/linux/6502 -c test00.asm -r 4000 -a 022a:55
Assert $022a:55=00 false
```

### Debugging Failing Tests

#### 1. Run with Trace
```bash
6502 -c test.asm -r 4000 -t
```

#### 2. Use the Debugger
```bash
6502 -c test.asm -d 4000
```

Then use debugger commands:
```
> step         # Execute one instruction
> registers    # View register state
> memory 8000 8010  # Examine memory
> list 4000 4020    # Disassemble code
```

#### 3. Dump State on Exit
```bash
6502 -c test.asm -r 4000 -prfsm
```
Flags: r=registers, f=flags, s=stack, m=memory

### Current Test Status

As of the latest update:

#### Passing (9 tests)
- ADCI, LDAI1, LDAI2, LDAI3
- CLC, SEC, NOP
- PHA, test05
- timing

#### Failing (2 tests)
- **test00**: Program logic doesn't produce expected result
- **test01**: Program stores result at unexpected address

### Best Practices

1. **Test one thing**: Each test should focus on specific functionality
2. **Use consistent addresses**: Start at `$4000`, store results at `$8000+`
3. **Keep tests simple**: Easier to debug and maintain
4. **Document expectations**: Add comments explaining what's being tested
5. **Test edge cases**: Boundary values, flag conditions, special states
6. **Use descriptive names**: Name tests after what they verify
7. **Verify flag behavior**: Test that flags are set/cleared correctly
8. **Test error conditions**: Invalid opcodes, boundary violations

### Test Development Workflow

1. **Write the test**: Create `.asm` file with test code
2. **Add makefile target**: Define test execution command
3. **Run manually first**: Verify basic functionality
```bash
6502 -c mytest.asm -r 4000 -t -prfsm
```
4. **Add assertion**: Determine correct result address and value
5. **Add to test suite**: Include in main test target
6. **Verify**: Run test suite to ensure no regressions
7. **Document**: Update test/README.md if needed

### Test Timing

The timing test validates CPU clock rate simulation:

```bash
make test-timing
```

This test:
- Runs a program with known cycle count at various clock rates (1-10 Hz)
- Measures actual execution time
- Compares against expected time (within tolerance)
- Validates that timing simulation is accurate

### Continuous Integration

To integrate tests into CI:
```bash
#!/bin/bash
cd 6502
make TYPE=release clean all
cd test
make test || exit 1
echo "All tests passed"
```

---

## Contributing

When contributing to this project:

1. **Follow existing code style**: Match the formatting and conventions
2. **Write tests**: Add tests for new instructions or features
3. **Update documentation**: Keep this guide and other docs current
4. **Test thoroughly**: Run full test suite before submitting
5. **Check for regressions**: Ensure existing tests still pass
6. **Document limitations**: Note any known issues or TODOs

## Getting Help

- Check the [README.md](README.md) for project overview
- See [BUILDING.md](BUILDING.md) for build instructions
- See [CHEATSHEET.md](CHEATSHEET.md) for usage reference
- Review [TODO.md](TODO.md) for known issues and planned work
- Look at test files in `/test` for examples
- Examine source code comments and doxygen annotations

## Additional Resources

- **6502.org**: Comprehensive 6502 resources and documentation
- **Visual 6502**: Interactive transistor-level simulation
- **NesDev Wiki**: NES development (6502 variant) information
- **Commodore 64 Programmer's Reference**: Classic 6502 programming guide
