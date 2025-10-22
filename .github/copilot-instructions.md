# GitHub Copilot Instructions for 6502 Emulator Project

This file provides context and guidance for GitHub Copilot when working with this repository.

## Project Overview

This is a 6502 CPU emulator written in C++ that includes:
- A 6502 CPU emulator core with instruction set implementation
- A basic assembler with parser
- A debugger with breakpoints and step-through capability
- An interactive command-line interface
- Unit tests for validation

The project is under active development and supports basic assembly, execution, and debugging of 6502 programs.

## Build System

### Technology Stack
- **Language**: C++ (compiled with GCC)
- **Build System**: GNU Make with platform detection
- **Platforms**: Linux (x86_64, ARM64) and macOS (Intel x86_64, Apple Silicon ARM64)
- **Build Types**: debug, release, profile

### Building the Project
```bash
# Build with automatic platform detection
make TYPE=debug

# Or explicitly specify platform
make PLATFORM=linux TYPE=debug

# Run tests
make test
```

### Key Build Files
- `makefile`: Main build configuration defining targets
- `include.mk`: Build system core with platform detection and compiler flags
- `test/makefile`: Test runner makefile

## Code Organization

### Main Source Files
- `main.cpp`: Command-line interface and entry point
- `l6502.cpp` / `l6502.h`: Core emulator library (CPU, memory, instruction set, assembler, debugger)
- `ftrace.cpp` / `ftrace.h`: Function tracing/debugging utility (enable with `export FTRACE=1`)
- `ticker.cpp` / `ticker.h`: CPU timing/clock simulation
- `util.cpp` / `util.h`: Utility functions (string manipulation, hex conversion)
- `platform.h`: Platform-specific definitions for cross-platform compatibility

### Directory Structure
```
6502/
├── bin/TYPE/PLATFORM/     # Binary outputs
├── lib/TYPE/PLATFORM/     # Library outputs
├── obj/TYPE/PLATFORM/     # Object files
├── test/                  # Unit tests
│   ├── *.asm             # Test assembly files
│   └── makefile          # Test runner
└── *.cpp, *.h            # Source files
```

## Coding Conventions

### Style Guidelines
- **Comments**: Match existing style; use sparingly and only when necessary to explain complex logic
- **Formatting**: Follow existing indentation and bracket placement
- **Naming**: Use descriptive names; existing code uses camelCase for variables and PascalCase for types

### Important Implementation Patterns

#### Instruction Implementation
Instructions use a macro-based pattern:
```cpp
INSTRUCTION(MNEMONIC, opcode, size, cycles, "Description")
{
    // 1. Fetch operand(s) from memory if needed
    // 2. Perform the operation
    // 3. Update affected registers
    // 4. Set/clear appropriate flags
    // 5. Advance PC by instruction size
    // 6. Add CPU cycles
}
```

#### Flag Handling
```cpp
setZero(result == 0);        // Z flag
setSign(result & 0x80);      // N flag (bit 7)
setCarry(carry_occurred);    // C flag
setOverflow(v_occurred);     // V flag
```

## Assembler Syntax (Non-Standard!)

**Important**: This assembler uses **non-standard 6502 syntax** with instruction suffixes to denote addressing modes.

### Key Syntax Rules
1. **Hexadecimal addresses**: Must use `$` prefix (e.g., `$8000`)
2. **Immediate values**: Must use `#` prefix
3. **Hexadecimal immediate values**: Must use both `#` and `$` (e.g., `#$FF`, not `#FF`)
4. **Addressing mode suffixes**: Different mnemonics for different modes (non-standard!)

### Addressing Mode Examples
```asm
LDAI #$42     ; Load accumulator immediate
LDAA $8000    ; Load accumulator from absolute address
LDAZ $80      ; Load accumulator from zero page
LDXI #$10     ; Load X register immediate
LDYI #$20     ; Load Y register immediate
```

Standard 6502 assemblers use the same mnemonic (e.g., `LDA`) with mode determined by operand format. This assembler is different!

### Common Instructions
```asm
; Load/Store
LDAI #$42, LDAA $8000, STAA $8000, STAZ $80

; Transfers
TAX, TAY, TXA, TYA, TXS, TSX

; Stack
PHA, PLA, PHP, PLP

; Arithmetic
ADCI #$10, SBCI #$05

; Increment/Decrement
INX, INY, DEX, DEY

; Flags
CLC, SEC, CLD, SED, CLI, SEI, CLV

; Control
JMP label, BRK, NOP
```

## Testing

### Test Infrastructure
- Tests are in the `/test` directory
- Each test is a `.asm` file that exercises specific functionality
- Tests use the `-a` flag to assert expected memory values

### Test Structure
A test should:
1. Start at a consistent address (convention: `$4000`)
2. Perform operations to test specific functionality
3. Store result at a predictable memory location (convention: `$8000+`)
4. End with `BRK` to halt execution

Example:
```asm
$4000   LDAI #$7f      ; Load value $7f
        STAA $8000     ; Store at test result address
        BRK            ; Halt
```

### Running Tests
```bash
# Run all tests
cd test && make test

# Run individual test
make test-LDAI1

# Run test manually with trace
../bin/debug/linux/6502 -c LDAI1.asm -r 4000 -t -a 8000:7f
```

### Test Assertions
Format: `-a <address>:<expected_value>`
- Returns exit code 0 on pass, 1 on fail
- Prints: `Assert $8000:7f=7f true` or `Assert $8000:7f=XX false`

### Adding New Tests
1. Create test `.asm` file in `/test`
2. Add makefile target in `test/makefile`:
   ```makefile
   test-mytest:
       @echo "Test mytest"
       $(EMU) -c mytest.asm -r 4000 -a 8000:42
   ```
3. Add to `test` target's dependencies
4. Run with `make test-mytest`

## Debugging

### Debug Flags
```bash
# Run with trace
6502 -c program.asm -r 4000 -t

# Run in debugger
6502 -c program.asm -d 4000

# Dump state on exit (-p followed by r=registers, f=flags, s=stack, m=memory)
6502 -c program.asm -r 4000 -prfsm

# Enable function tracing
export FTRACE=1
6502 -c program.asm -r 4000
```

### Debugger Commands
- `help` - Show commands
- `run` - Continue execution
- `step` - Execute one instruction
- `list [start] [end]` - Disassemble memory
- `break <address>` - Set breakpoint
- `clear <address>` - Clear breakpoint
- `registers` - Display registers
- `flags` - Display status flags
- `memory [start] [end]` - Dump memory
- `quit` - Exit

## Memory Organization

The emulator provides a 64KB address space:
```
$0000 - $00FF    Zero Page (256 bytes)
$0100 - $01FF    Stack (256 bytes, grows downward from $01FF)
$0200 - $3FFF    General RAM
$4000 - $FFFF    General RAM / Program space

Common conventions:
$4000            Default program start address
$8000+           Test result storage area
```

## CPU State

### Registers (6502)
- **A**: Accumulator (8-bit)
- **X**: Index Register X (8-bit)
- **Y**: Index Register Y (8-bit)
- **PC**: Program Counter (16-bit)
- **SP**: Stack Pointer (8-bit, points into $0100-$01FF)
- **P**: Processor Status (8-bit flags)

### Status Flags (P register)
- **Bit 0 (C)**: Carry flag
- **Bit 1 (Z)**: Zero flag
- **Bit 2 (I)**: Interrupt disable flag
- **Bit 3 (D)**: Decimal mode flag
- **Bit 4 (B)**: Break flag
- **Bit 5**: Unused (always 1)
- **Bit 6 (V)**: Overflow flag
- **Bit 7 (N)**: Negative/Sign flag

## Known Issues and Limitations

1. **Assembler is non-standard**: Uses suffixed mnemonics instead of standard 6502 syntax
2. **Hex format requirement**: Immediate hex values need both `#` and `$` (e.g., `#$FF`)
3. **Test coverage is incomplete**: Some tests fail (e.g., test00)
4. **Parser robustness**: Sensitive to formatting, limited error messages
5. **Comment parsing**: May be inconsistent in assembly files

See `TODO.md` for full list of planned improvements.

## Contributing Guidelines

When making changes:
1. **Follow existing patterns**: Match instruction implementation style
2. **Write tests**: Add tests for new instructions or features
3. **Test thoroughly**: Run `make test` before submitting
4. **Update documentation**: Keep `DEVELOPMENT.md` and other docs current
5. **Check for regressions**: Ensure existing tests still pass
6. **Minimal changes**: Only modify what's necessary to fix/implement the feature
7. **Build on both platforms**: Test on Linux and macOS if possible

## References

- See `DEVELOPMENT.md` for comprehensive development guide
- See `BUILDING.md` for build instructions
- See `CHEATSHEET.md` for usage reference
- See `TODO.md` for known issues and planned work
- Official 6502 documentation: http://www.6502.org/

## Special Considerations for AI Assistance

1. **Assembler syntax is unique**: Don't assume standard 6502 assembler conventions
2. **Test first**: When adding instructions, write tests before implementation
3. **Instruction table**: New instructions need both implementation and MAP() call
4. **Platform differences**: Code must work on both Linux and macOS
5. **Build artifacts**: Don't commit files in `bin/`, `lib/`, or `obj/` directories
6. **Tests may take time**: Timing tests can run for several minutes
