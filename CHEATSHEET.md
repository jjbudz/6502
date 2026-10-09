The 6502 program supports a small two-pass assembler, loader, and debugger. The
assembler accepts standard 6502 syntax (`LDA $1234,X`) and a legacy syntax in which
the addressing mode is a suffix on the mnemonic (`LDAX $1234`); the two can be
mixed. Use the "-i" flag to get a list of the legacy instruction names.

Hexadecimal values must be prefixed with $; decimal values are bare digits:

```
  STA $8000
  STA $80
  LDA #59
  LDA #$5A
```

Jump and branch destinations may be labels or literal addresses. A label is an
identifier in the first column, or any identifier followed by a colon:

```asm
  foo JMP bar
      LDA #00
      STA $8000
      BRK
  bar: LDA #01
      STA $8000
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

With a bare mnemonic the operand's shape selects the addressing mode, as in any
6502 assembler:

```asm
LDA #$42         ; Immediate
LDA $80          ; Zero page ($80 has fewer than four digits and is under $100)
LDA $80,X        ; Zero page,X
LDX $80,Y        ; Zero page,Y
LDA $8000        ; Absolute ($0080 would also be absolute: four digits)
LDA $8000,X      ; Absolute,X
LDA $8000,Y      ; Absolute,Y
LDA ($40,X)      ; (Indirect,X): pointer at $40+X
LDA ($40),Y      ; (Indirect),Y: pointer at $40, plus Y
JMP ($1234)      ; (Indirect) jump through the pointer at $1234
LSR A            ; Accumulator
BNE loop         ; Relative branch to a label
BEQ $4010        ; Relative branch to a literal address
```

A label used before it is defined is encoded as an absolute address. Using a mode
an instruction does not have (`LDX $10,X`) is an error.

### Expressions, Constants and Directives

```asm
BASE = $50               ; constant
MASK = %00001111         ; binary literal
* = $4000                ; same as .ORG $4000
        LDA #'A'         ; character literal ($41)
        AND #MASK
        STA BASE+2       ; expressions use + and -, 16-bit
        LDA #<TARGET     ; low byte of an address
        LDX #>TARGET     ; high byte
        JMP *+6          ; * is this instruction's address
        LDA #-1          ; negative immediate ($FF)
TABLE   .BYTE 1, 2, 3    ; one byte each
PTR     .WORD $1234, TABLE ; two bytes each, low first
MSG     .TEXT "Hello", 0 ; string bytes (case kept) plus a terminator
```

The legacy syntax instead spells the mode in the mnemonic:

```asm
; Immediate addressing - suffix 'I'
LDAI #$42        ; Load accumulator with immediate value $42
LDXI #10         ; Load X register with decimal 10
LDYI #$FF        ; Load Y register with $FF

; Absolute addressing - suffix 'A'
LDAA $8000       ; Load accumulator from address $8000
STAA $8001       ; Store accumulator to address $8001
JMP $4020        ; Jump to absolute address $4020 (JMP has no suffix)

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

