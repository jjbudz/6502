# 6502 Emulator Test Suite

## Running Tests

To run the test suite:

```bash
cd test
PATH=$PATH:../bin/debug/ubuntu_x86-64 bash unittest.script
```

Note: Adjust the path based on your platform (e.g., `macos_arm64`, `win32_x86`, etc.)

## Test Status

### Currently Passing Tests (6 tests)

1. **LDAI1** - Load accumulator immediate with value $00, then $7f
2. **LDAI2** - Load accumulator immediate with value $7f
3. **LDAI3** - Load accumulator immediate with value $00
4. **PHA** - Push accumulator to stack (value $ff)
5. **test05** - Complex multi-instruction test
6. **timing** - Timing simulation validation test (uses ticker functions)

### Disabled Tests (6 tests)

#### Parser Limitation Issues (4 tests)
These tests use hex notation without the `$` prefix (e.g., `#FF` instead of `#$FF`):
- **CLC** - Clear carry flag test
- **SEC** - Set carry flag test
- **NOP** - No operation test
- **test01** - Complex logical operations test

**Fix Options:**
1. Update the parser to accept both `#FF` and `#$FF` formats
2. Update test files to use `#$FF` format

#### Test Script Issue (1 test)
- **ADCI** - Add with carry test
  - Issue: Test script expects value `ff` but program writes `01` on success
  - Fix: Change expected value in unittest.script from `ff` to `01`

#### Test Logic Error (1 test)
- **test00** - Complex addressing mode test
  - Issue: Test expects value $55 at address $022A, but program stores at $0200
  - Fix: Debug and correct the test00.asm program logic

## Adding New Tests

To add a new test:

1. Create a `.asm` file in the test directory
2. Add an echo statement and test command to `unittest.script`
3. Use the `-a` flag to specify expected memory address:value pair
4. Example: `6502 -c mytest.asm -r 4000 -a 8000:42`

## Test File Format

Test files should:
- Start at address $4000 by convention
- End with a BRK instruction
- Store test result at a predictable memory location
- Use proper hex notation (`#$XX` for immediate hex values)
