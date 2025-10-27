The 6502 program supports a barebone assembler, loader, and debugger. However, the
assembler's parser is a bit rough and requires different symbols to access each of
the various addressing modes. Use the "-i" flag to get a list of known instructions.

Hexidecimal address values must be prefixed with $, e.g.:

```
  STAA $8000
  STAZ $80
```

Immediate values are denoted using #, e.g.:

```asm
  LDAI #59
  LDAI #$5A
```

Jumps and branch destinations may be specified using labels as follows:

```asm
  foo JMP bar
      LDAI #00
      STAA $8000
      BRK
  bar LDAI #01
      STAA $8000
      BRK
```

Sample syntax for the assembler can be found by looking at the .asm files
in the [tests/](tests) directory.

Program command line arguments include:

```
  -l <filename> -a <filename> -s <filename> -r [<address>] [-t] [-p] where:
  -h to display command line options
  -l <filename> to load an object file
  -c <filename> to compile source file
  -s <filename> to save object file after assembly
  -r <address> to run code from the address (hexadecimal, e.g. A000)
  -d <address> to debug code from the address (hexadecimal, e.g. A000)
  -a <address>:<value> to assert value matches at the given address
  -t to turn on trace output
  -i to list assembler instructions
  -p[rfsm] to print (dump) registers, flags, stack, and memory on exit
  -v to print version information
  --rate <hz> to set CPU clock rate in Hz (default: 1000000)
```

Command line examples:

```bash
  # Display version information
  6502 -v
  
  # List all available assembler instructions
  6502 -i
  
  # Compile and run a program, assert result, print registers
  6502 -c LDAI2.asm -r 4000 -a 8000:7f -pr
  
  # Compile and debug a program starting at address $4000
  6502 -c LDAI2.asm -d 4000
  
  # Compile source and save to object file
  6502 -c LDAI2.asm -s LDAI2.6502
  
  # Load object file and run with assertion, dump stack, flags, and registers
  6502 -l LDAI2.6502 -r 4000 -a 8000:7f -psfr
  
  # Run with trace output enabled
  6502 -c sample.asm -r 4000 -t
  
  # Set CPU clock rate to 2 MHz (2,000,000 Hz)
  6502 -c program.asm -r 4000 --rate 2000000
  
  # Run at original 6502 speed (1.79 MHz, similar to Apple II)
  6502 -c program.asm -r 4000 --rate 1790000
  
  # Dump all state (registers, flags, stack, memory) on exit
  6502 -c program.asm -r 4000 -prfsm
```

## Assembler Syntax Examples

### Addressing Modes

The assembler uses non-standard instruction suffixes to denote addressing modes:

```asm
; Immediate addressing - suffix 'I'
LDAI #$42        ; Load accumulator with immediate value $42
LDXI #10         ; Load X register with decimal 10
LDYI #$FF        ; Load Y register with $FF

; Absolute addressing - suffix 'A'
LDAA $8000       ; Load accumulator from address $8000
STAA $8001       ; Store accumulator to address $8001
JMPA $4020       ; Jump to absolute address $4020

; Zero page addressing - suffix 'Z'
LDAZ $80         ; Load accumulator from zero page $80
STAZ $90         ; Store accumulator to zero page $90

; Zero page indexed - suffix 'ZX' or 'ZY'
LDAZX $80        ; Load accumulator from ($80 + X)
STAZX $90        ; Store accumulator to ($90 + X)
LDXZY $50        ; Load X from ($50 + Y)

; Absolute indexed - suffix 'X' or 'Y'
LDAX $8000       ; Load accumulator from ($8000 + X)
LDAY $8000       ; Load accumulator from ($8000 + Y)
STAX $9000       ; Store accumulator to ($9000 + X)

; Indirect indexed - suffix 'IX' or 'IY'
LDAIX $40        ; Load accumulator from (($40 + X))
LDAIY $50        ; Load accumulator from (($50)) + Y
```

### Complete Program Examples

#### Example 1: Simple counter loop

```asm
; Count down from 5 to 0 using X register
$4000   LDXI #$05        ; Load X with 5
LOOP    DEX              ; Decrement X
        BNE LOOP         ; Branch if not zero
        STXA $8000       ; Store final value (0)
        BRK              ; Halt
```

#### Example 2: Using labels and branches

```asm
; Test if accumulator equals a value
$4000   LDAI #$42        ; Load test value
        CMPI #$42        ; Compare with $42
        BEQ EQUAL        ; Branch if equal
        LDAI #$00        ; Not equal - load 0
        JMP DONE
EQUAL   LDAI #$FF        ; Equal - load $FF
DONE    STAA $8000       ; Store result
        BRK
```

#### Example 3: Data definitions and memory operations

```asm
; Define data in memory
$40     .DATA $12 $34 $56 $78

; Copy data from one location to another
$4000   LDAZ $40         ; Load from zero page $40
        STAA $8000       ; Store to $8000
        LDAZ $41         ; Load from $41
        STAA $8001       ; Store to $8001
        BRK
```

#### Example 4: Stack operations

```asm
; Demonstrate stack usage
$4000   LDAI #$AA        ; Load accumulator
        PHA              ; Push to stack
        LDAI #$55        ; Change accumulator
        PLA              ; Pull from stack (A=$AA again)
        STAA $8000       ; Store result
        BRK
```

#### Example 5: Arithmetic with carry

```asm
; Add two numbers with carry
$4000   CLC              ; Clear carry flag
        LDAI #$50        ; Load first value
        ADCI #$60        ; Add second value
        STAA $8000       ; Store sum ($B0)
        BRK
```

## Debugger Usage

When using the `-d` flag, the program enters interactive debug mode:

```bash
6502 -c program.asm -d 4000
```

Available debugger commands:

```
  help (or h)              - Display this help
  run (or r)               - Continue execution until breakpoint or halt
  step (or s)              - Execute one instruction
  go (or g)                - Continue execution
  registers (or e)         - Display CPU registers
  flags (or f)             - Display status flags
  stack (or a)             - Display stack contents
  list (or l) <start> <end> - Disassemble memory range
  print (or p) <start> <end> - Dump memory range
  break (or b) <address>   - Set breakpoint at address
  clear (or c) <address>   - Clear breakpoint at address
  trace (or t)             - Toggle trace mode
  quit (or q)              - Exit debugger
```

Example debugging session:

```
> registers
A:00 X:00 Y:00 SP:FF PC:4000
> step
$4000: A9 42    LDAI #$42
> registers
A:42 X:00 Y:00 SP:FF PC:4002
> break 4010
Breakpoint set at $4010
> run
Breakpoint at $4010
> quit
```

## Additional Tips

- All hexadecimal addresses must use the `$` prefix
- Immediate hex values require both `#` and `$` (e.g., `#$FF`)
- Labels can be placed on the same line as instructions
- The assembler is case-sensitive
- Comments start with `;` character
- Test programs typically store results at `$8000` and above
- Default program start address is `$4000`
- Stack is located at `$0100-$01FF` and grows downward

