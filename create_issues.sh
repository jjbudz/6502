#!/bin/bash
# Script to create GitHub issues from FAILING_TESTS_ISSUES.md
# This script requires gh CLI to be authenticated with: gh auth login

set -e

echo "Creating GitHub issues for failing unit tests..."
echo ""

# Issue 1: ASLZX
gh issue create \
  --title "ASLZX instruction uses incorrect addressing mode" \
  --body "## Bug Description
The ASLZX (Arithmetic Shift Left Zero Page, X) instruction implementation uses indirect addressing instead of zero page indexed addressing.

## Location
\`l6502.cpp\`, \`INSTRUCTION(ASLZX, 0x16, 2, 6)\`

## Current Implementation
\`\`\`cpp
uint8_t* addr = BP + *(BP+zx) + X;
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t* addr = BP + zx;
\`\`\`

## Test Failure
The unit test \`test-ASLZX\` expects result \`0xAA\` but gets \`0x55\`.

## Expected Behavior
ASLZX should perform arithmetic shift left on the value at zero page address (operand + X register), not use indirect addressing.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-ASLZX
\`\`\`

The test will fail with: \`Assert \$8000:aa=55 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 1/11: ASLZX"

# Issue 2: ROLZX
gh issue create \
  --title "ROLZX instruction uses incorrect addressing mode" \
  --body "## Bug Description
The ROLZX (Rotate Left Zero Page, X) instruction implementation uses indirect addressing instead of zero page indexed addressing.

## Location
\`l6502.cpp\`, \`INSTRUCTION(ROLZX, 0x36, 2, 6)\`

## Current Implementation
\`\`\`cpp
uint8_t* addr = BP + *(BP+zx) + X;
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t* addr = BP + zx;
\`\`\`

## Test Failure
The unit test \`test-ROLZX\` expects result \`0xAA\` but gets \`0x55\`.

## Expected Behavior
ROLZX should perform rotate left on the value at zero page address (operand + X register), not use indirect addressing.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-ROLZX
\`\`\`

The test will fail with: \`Assert \$8000:aa=55 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 2/11: ROLZX"

# Issue 3: CMPA
gh issue create \
  --title "CMPA instruction uses addition instead of subtraction" \
  --body "## Bug Description
The CMPA (Compare Accumulator with Absolute Memory) instruction uses addition instead of subtraction for comparison.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CMPA, 0xCD, 3, 4)\`

## Current Implementation
\`\`\`cpp
uint8_t a = A + *(BP + addr16);
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t a = A - *(BP + addr16);
\`\`\`

## Test Failure
The unit test \`test-CMPA\` expects the comparison to set the zero flag when values are equal, but it doesn't.

## Expected Behavior
Compare instructions should perform subtraction (A - M) to set the appropriate flags for comparison, not addition.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CMPA
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 3/11: CMPA"

# Issue 4: CMPIY
gh issue create \
  --title "CMPIY instruction has incorrect carry flag logic" \
  --body "## Bug Description
The CMPIY (Compare with Indirect Indexed Y) instruction's carry flag logic is incorrect - it checks the sign bit instead of proper carry from comparison.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CMPIY, 0xF1, 2, 5)\`

## Current Implementation
\`\`\`cpp
SET_CARRY(((a&0x80)==0x80));
\`\`\`

## Expected Behavior
The carry flag should be set if A >= M (no borrow occurred), which means the comparison result should be >= 0 when treating values as unsigned. The current implementation only checks if the result's sign bit is set.

## Test Failure
The unit test \`test-CMPIY\` fails comparison assertions.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CMPIY
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 4/11: CMPIY"

# Issue 5: CMPX
gh issue create \
  --title "CMPX instruction has incorrect carry flag logic" \
  --body "## Bug Description
The CMPX (Compare with Absolute, X) instruction's carry flag logic is incorrect.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CMPX, 0xFD, 3, 4)\`

## Current Implementation
\`\`\`cpp
SET_CARRY(((a&0x80)==0x80));
\`\`\`

## Expected Behavior
The carry flag should be set if A >= M (no borrow occurred), which means the comparison result should be >= 0 when treating values as unsigned.

## Test Failure
The unit test \`test-CMPX\` fails comparison assertions.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CMPX
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 5/11: CMPX"

# Issue 6: CPXA
gh issue create \
  --title "CPXA instruction uses addition instead of subtraction" \
  --body "## Bug Description
The CPXA (Compare X with Absolute Memory) instruction uses addition instead of subtraction.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CPXA, 0xEC, 3, 4)\`

## Current Implementation
\`\`\`cpp
uint8_t x = X + *(BP + addr16);
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t x = X - *(BP + addr16);
\`\`\`

## Test Failure
The unit test \`test-CPXA\` expects the comparison to set the zero flag when values are equal, but it doesn't.

## Expected Behavior
Compare instructions should perform subtraction (X - M) to set the appropriate flags for comparison, not addition.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CPXA
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 6/11: CPXA"

# Issue 7: CPYA
gh issue create \
  --title "CPYA instruction uses addition instead of subtraction" \
  --body "## Bug Description
The CPYA (Compare Y with Absolute Memory) instruction uses addition instead of subtraction.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CPYA, 0xCC, 3, 4)\`

## Current Implementation
\`\`\`cpp
uint8_t y = Y + *(BP + addr16);
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t y = Y - *(BP + addr16);
\`\`\`

## Test Failure
The unit test \`test-CPYA\` expects the comparison to set the zero flag when values are equal, but it doesn't.

## Expected Behavior
Compare instructions should perform subtraction (Y - M) to set the appropriate flags for comparison, not addition.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CPYA
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 7/11: CPYA"

# Issue 8: CPYI
gh issue create \
  --title "CPYI instruction sets flags on wrong variable" \
  --body "## Bug Description
The CPYI (Compare Y Immediate) instruction calculates the comparison correctly but sets the zero and sign flags on the Y register instead of the comparison result.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CPYI, 0xC0, 2, 2)\`

## Current Implementation
\`\`\`cpp
uint8_t y = Y - *(BP+PC+1);
SET_CARRY(((y&0x80)==0x80));
SET_ZERO(Y);  // Wrong - should be y
SET_SIGN(Y);  // Wrong - should be y
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t y = Y - *(BP+PC+1);
SET_CARRY(((y&0x80)==0x80));
SET_ZERO(y);
SET_SIGN(y);
\`\`\`

## Test Failure
The unit test \`test-CPYI\` fails comparison assertions.

## Expected Behavior
The flags should reflect the result of the comparison (Y - immediate value), not the Y register itself.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CPYI
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 8/11: CPYI"

# Issue 9: CPYZ
gh issue create \
  --title "CPYZ instruction has incorrect borrow logic" \
  --body "## Bug Description
The CPYZ (Compare Y with Zero Page) instruction incorrectly subtracts the borrow (carry) like SBC instruction instead of performing a pure comparison.

## Location
\`l6502.cpp\`, \`INSTRUCTION(CPYZ, 0xC4, 2, 3)\`

## Current Implementation
\`\`\`cpp
uint8_t y = Y - *(BP+*(BP+PC+1)) - (1 - CARRYBIT);
\`\`\`

## Expected Implementation
\`\`\`cpp
uint8_t y = Y - *(BP+*(BP+PC+1));
\`\`\`

## Test Failure
The unit test \`test-CPYZ\` fails comparison assertions.

## Expected Behavior
Compare instructions should perform simple subtraction (Y - M) without considering the carry flag. The carry flag is only used in SBC (subtract with carry) instructions, not compare instructions.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-CPYZ
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 9/11: CPYZ"

# Issue 10: JMPI
gh issue create \
  --title "JMPI instruction calculates indirect address incorrectly" \
  --body "## Bug Description
The JMPI (Jump Indirect) instruction's \`getIndirectAddress()\` function reads from the wrong memory location, causing jumps to incorrect addresses.

## Location
\`l6502.cpp\`, \`getIndirectAddress()\` function

## Current Implementation
\`\`\`cpp
uint16_t getIndirectAddress()
{
    uint16_t pc = getAbsoluteAddress();
    return (*(BP+pc+1)<<8) + *(BP+pc); 
}
\`\`\`

## Issue
The function is reading the target address from \`BP+pc\` and \`BP+pc+1\`, but \`pc\` already contains the address from the instruction operand. This causes it to read from the address specified in the operand, plus an offset, instead of reading the 16-bit address stored at the operand location.

## Test Failure
The unit test \`test-JMPI\` attempts to jump to address \`0x4010\` but ends up jumping to \`0x0000\`.

Test setup:
- Stores \`0x10\` at zero page address \`0x50\`
- Stores \`0x40\` at zero page address \`0x51\`
- Executes \`JMPI \$50\` which should read the 16-bit address \`0x4010\` and jump there

## Expected Behavior
JMPI should read a 16-bit address from the location specified by the operand and jump to that address. For \`JMPI \$50\`, it should read the low byte from \`\$50\` and high byte from \`\$51\`, forming the target address.

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-JMPI
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator"

echo "✓ Created issue 10/11: JMPI"

# Issue 11: RTI
gh issue create \
  --title "RTI instruction test fails - may need proper interrupt context" \
  --body "## Description
The RTI (Return from Interrupt) instruction test fails, jumping to the wrong address. This may be a test design issue rather than an emulator bug.

## Location
\`test/RTI.asm\`

## Test Code
\`\`\`asm
\$4000   SEC
        PHP
        CLI
        SEI
        RTI
        LDAI #\$01
        STAA \$8000
        BRK
\`\`\`

## Issue
The test uses PHP to push the processor status, then modifies flags, and calls RTI. However, RTI is designed to return from an interrupt handler, which would have pushed both the processor status and return address onto the stack. The current test doesn't set up a proper interrupt context (no return address on stack).

## Test Failure
The unit test \`test-RTI\` jumps to address \`0x0001\` instead of continuing execution.

## Expected Behavior
RTI should:
1. Pull processor status from stack
2. Pull program counter (low byte then high byte) from stack
3. Resume execution at the restored PC address

## Possible Solutions
1. Modify the test to properly simulate an interrupt by pushing a return address before calling RTI
2. Investigate if the RTI implementation correctly handles the stack operations
3. Determine if this is a known limitation of testing RTI outside of actual interrupt context

## Steps to Reproduce
\`\`\`bash
make PLATFORM=linux TYPE=debug
cd test && make PLATFORM=linux TYPE=debug test-RTI
\`\`\`

The test will fail with: \`Assert \$8000:01=00 false\`" \
  --label "bug,emulator,question"

echo "✓ Created issue 11/11: RTI"

echo ""
echo "✅ Successfully created all 11 GitHub issues!"
