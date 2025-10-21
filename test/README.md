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
