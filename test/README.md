# 6502 Emulator Test Suite

## Running Tests

### Using Make (Recommended)

To run all tests from the project root:

```bash
make PLATFORM=ubuntu_x86-64 TYPE=debug test
```

Or from the test directory:

```bash
cd test
make PLATFORM=ubuntu_x86-64 TYPE=debug test
```

To run individual tests:

```bash
cd test
make PLATFORM=ubuntu_x86-64 TYPE=debug test-LDAI1
make PLATFORM=ubuntu_x86-64 TYPE=debug test-PHA test-test05
```

Available test targets:
- `test-ADCI` - Add with carry immediate test
- `test-LDAI1`, `test-LDAI2`, `test-LDAI3` - Load accumulator immediate tests
- `test-CLC` - Clear carry flag test
- `test-SEC` - Set carry flag test
- `test-NOP` - No operation test
- `test-PHA` - Push accumulator to stack test
- `test-test00`, `test-test01`, `test-test05` - Complex multi-instruction tests
- `test-timing` - Timing simulation validation test

Note: Adjust the PLATFORM based on your system (e.g., `macos_arm64`, `win32_x86`, etc.)

### Using unittest.script (Legacy)

You can still run the legacy test script:

```bash
cd test
PATH=$PATH:../bin/debug/ubuntu_x86-64 bash unittest.script
```

Or override the emulator command:

```bash
cd test
EMU_CMD=/path/to/your/6502 bash unittest.script
```

## Test Status

### Summary
- **Total tests**: 197 (188 enabled, 9 disabled)
- **Passing tests**: 188
- **Failing tests**: 0
- **Disabled tests**: 9 (BCD arithmetic, see below)

### Currently Passing Tests (188 tests)

#### ADC (Add with Carry) - 8 passing
- ADCA, ADCI, ADCIX, ADCIY, ADCX, ADCY, ADCZ, ADCZX

#### AND (Bitwise AND) - 8 passing
- ANDA, ANDI, ANDIX, ANDIY, ANDX, ANDY, ANDZ, ANDZX

#### ASL (Arithmetic Shift Left) - 5 passing
- ASL, ASLA, ASLX, ASLZ, ASLZX

#### Branch Instructions - 8 passing
- BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS

#### BIT (Bit Test) - 2 passing
- BIT, BITZ

#### BRK (Break) - 1 passing
- BRK

#### Clear Flag Instructions - 4 passing
- CLC, CLD, CLI, CLV

#### CMP (Compare Accumulator) - 8 passing
- CMPA, CMPI, CMPIX, CMPIY, CMPX, CMPY, CMPZ, CMPZX

#### CPX (Compare X Register) - 3 passing
- CPXA, CPXI, CPXZ

#### CPY (Compare Y Register) - 3 passing
- CPYA, CPYI, CPYZ

#### DEC (Decrement) - 6 passing
- DECA, DECX, DECZ, DECZX, DEX, DEY

#### EOR (Exclusive OR) - 8 passing
- EORA, EORI, EORIX, EORIY, EORX, EORY, EORZ, EORZX

#### INC (Increment) - 6 passing
- INCA, INCX, INCZ, INCZX, INX, INY

#### JMP (Jump) - 2 passing
- JMP, JMPI

#### JSR (Jump to Subroutine) - 1 passing
- JSR

#### LDA (Load Accumulator) - 10 passing
- LDAA, LDAI1, LDAI2, LDAI3, LDAIX, LDAIY, LDAX, LDAY, LDAZ, LDAZX

#### LDX (Load X Register) - 4 passing
- LDXA, LDXY, LDXZ, LDXZY

#### LDY (Load Y Register) - 4 passing
- LDYA, LDYX, LDYZ, LDYZX

#### LSR (Logical Shift Right) - 5 passing
- LSR, LSRA, LSRX, LSRZ, LSRZX

#### NOP (No Operation) - 1 passing
- NOP

#### ORA (Bitwise OR) - 8 passing
- ORAA, ORAI, ORAIX, ORAIY, ORAX, ORAY, ORAZ, ORAZX

#### Stack Operations - 4 passing
- PHA, PHP, PLA, PLP

#### ROL (Rotate Left) - 5 passing
- ROL, ROLA, ROLX, ROLZ, ROLZX

#### ROR (Rotate Right) - 5 passing
- ROR, RORA, RORX, RORZ, RORZX

#### RTI/RTS (Return) - 2 passing
- RTI, RTS

#### SBC (Subtract with Carry) - 8 passing
- SBCA, SBCI, SBCIX, SBCIY, SBCX, SBCY, SBCZ, SBCZX

#### Set Flag Instructions - 3 passing
- SEC, SED, SEI

#### STA (Store Accumulator) - 7 passing
- STAA, STAIX, STAIY, STAX, STAY, STAZ, STAZX

#### STX (Store X Register) - 3 passing
- STXA, STXZ, STXZY

#### STY (Store Y Register) - 3 passing
- STYA, STYZ, STYZX

#### Transfer Instructions - 6 passing
- TAX, TAY, TSX, TXA, TXS, TYA

#### Complex Tests - 3 passing
- test00 - Complex addressing mode test
- test01 - Complex logical operations test
- test05 - Complex multi-instruction test

#### Timing Test - 1 passing
- timing - Timing simulation validation test (uses ticker functions)

#### Phase 1 Edge Cases: Page Boundary Crossing - 6 passing
- LDAX-PAGE, LDAY-PAGE, STAIX-PAGE, LDAIY-PAGE, STAX-PAGE, STAY-PAGE

#### Phase 1 Edge Cases: Zero Page Wraparound - 6 passing
- LDAZX-WRAP, LDAZY-WRAP, STAZX-WRAP, STXZY-WRAP, STYZX-WRAP, ANDIX-WRAP

#### Phase 1 Edge Cases: JMP Indirect Page Boundary Bug - 3 passing
- JMPI-BUG-01, JMPI-BUG-02, JMPI-BUG-03

#### Phase 2 Edge Cases: Stack Operations - 5 passing
- STACK-WRAP-01, STACK-WRAP-02 - Stack pointer wraparound at $00/$FF
- STACK-JSR-01 - JSR/RTS with stack pointer near bottom
- STACK-PRESERVE-01 - PHP/PLP flag preservation
- STACK-FULL-01 - LIFO ordering across multiple push/pull

#### Phase 2 Edge Cases: Flag Interactions - 6 passing
- FLAG-ADC-NVZ - N, V and Z set together by one ADC
- FLAG-SBC-BORROW - Carry as inverted borrow in SBC
- FLAG-OVERFLOW-01, FLAG-OVERFLOW-02 - Signed overflow in both directions
- FLAG-ZERO-ADC, FLAG-ZERO-SBC - Zero flag from arithmetic wrap

#### Phase 2 Edge Cases: Boundary Values - 7 passing
- BOUNDARY-INC-FF, BOUNDARY-DEC-00 - Wrap at $FF/$00
- BOUNDARY-ADC-7F, BOUNDARY-SBC-80 - Sign bit transitions
- BOUNDARY-ASL-80, BOUNDARY-LSR-01, BOUNDARY-ROL-FF - Shift/rotate carry edges

### Disabled Tests (9 tests)

#### BCD (Decimal Mode) Arithmetic - 9 disabled
- BCD-ADC-01 through BCD-ADC-06, BCD-SBC-01 through BCD-SBC-03
- Reason: the emulator does not implement decimal mode (SED/CLD set the flag
  but ADC/SBC always perform binary arithmetic). Commented out in
  `run_tests.sh`; see `BCD-MODE-README.md`. Re-enable once BCD is implemented.

### Previously Failing Tests (Now Fixed)

- **ADCI** - Add with carry immediate test
  - Previous Issue: Test was reported as failing
  - Status: ✅ FIXED - Test now passes successfully

- **test00** - Complex addressing mode test
  - Previous Issue: Test expected value $55 at address $022A, but program stored at $0200
  - Status: ✅ FIXED - Corrected store address to match expected test location

- **SBC carry/overflow flags** (emulator bug found by Phase 2 tests)
  - Previous Issue: All SBC variants set C to the result's sign bit and V to
    C xor N, so C was clear after any non-negative subtraction and V was
    almost never correct. Results were right, so the original SBC tests
    (which only assert the result) never caught it.
  - Status: ✅ FIXED - SBC now computes C (no borrow) and V like ADC of the
    one's complement, verified by FLAG-SBC-BORROW, FLAG-ZERO-SBC and
    BOUNDARY-SBC-80

## Adding New Tests

To add a new test:

1. Create a `.asm` file in the test directory
2. Add a new test target to `test/makefile`:
   ```makefile
   test-mytest:
       @echo "Test mytest"
       $(EMU) -c mytest.asm -r 4000 -a 8000:42
   ```
3. Add the target name to the `.PHONY` list in `test/makefile`
4. Add a `run_test test-mytest` line to `run_tests.sh`
5. Use the `-a` flag to specify expected memory address:value pair
6. Run it before committing: `make test-mytest`

To debug a failing test, run the emulator directly with tracing and a
register/flag dump, which prints PC, opcode, A/X/Y/SP and every flag
after each instruction:

```bash
cd test
../bin/debug/linux/6502 -c mytest.asm -r 4000 -t -prf
```

## Test File Format

Test files should:
- Start at address $4000 by convention
- End with a BRK instruction
- Store test result at a predictable memory location
- Use proper hex notation (`#$XX` for immediate hex values)
- Use **labels** for branch targets (`BEQ pass` ... `pass    LDAI #$01`).
  A literal address such as `BEQ $400C` is assembled as a 16-bit absolute
  operand, but the CPU reads branches as 8-bit relative offsets, so the
  program will jump to the wrong place and usually crash on an unimplemented
  opcode.
- Prefer a success sentinel (store `$01` only after every check passes) over
  asserting a result of `$00`, since unwritten memory already reads as `$00`
  and an early `BRK` would then pass by accident.
