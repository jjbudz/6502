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
- **Total tests**: 154
- **Passing tests**: 153
- **Failing tests**: 1
- **Disabled tests**: 1

### Currently Passing Tests (153 tests)

#### ADC (Add with Carry) - 7 passing
- ADCA, ADCIX, ADCIY, ADCX, ADCY, ADCZ, ADCZX

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

#### Complex Tests - 2 passing
- test01 - Complex logical operations test
- test05 - Complex multi-instruction test

#### Timing Test - 1 passing
- timing - Timing simulation validation test (uses ticker functions)

### Currently Failing Tests (1 test)

- **ADCI** - Add with carry immediate test
  - Issue: Test fails at the second comparison, expected $8000:01 but got $8000:fe
  - Status: Test logic appears to have an issue that needs investigation

### Disabled Tests (1 test)

- **test00** - Complex addressing mode test
  - Issue: Test expects value $55 at address $022A, but program stores at $0200
  - Status: Disabled in makefile due to test logic issue

## Adding New Tests

To add a new test:

1. Create a `.asm` file in the test directory
2. Add a new test target to `test/makefile`:
   ```makefile
   test-mytest:
       @echo "Test mytest"
       $(EMU) -c mytest.asm -r 4000 -a 8000:42
   ```
3. Add the new test target to the `test` target's dependency list
4. Optionally add to `unittest.script` for legacy compatibility
5. Use the `-a` flag to specify expected memory address:value pair

## Test File Format

Test files should:
- Start at address $4000 by convention
- End with a BRK instruction
- Store test result at a predictable memory location
- Use proper hex notation (`#$XX` for immediate hex values)
