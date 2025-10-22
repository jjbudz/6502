A partial list of things that don't work or are in need of enhancement:

1. Unit tests - a bunch have been generated but more complex tests are needed.
1. Some non-nominal tests of the assembler would be useful.
1. The assembler/parser code is in bad need of refactoring
1. The assembler should be enhanced to not require explicit addressing/indexing modes.
1. Various @todos need to be addressed.

## Unit Test Failures Analysis

The following unit tests have been analyzed. Some are disabled due to issues that require significant parser or test refactoring:

### Failing Tests - Disabled

#### test00 Test
- **Status**: DISABLED (test logic error)
- **Issue**: Test expects value `55` at address $022A, but program stores it at $0200
- **Root Cause**: Test program logic doesn't produce the expected result at the expected address
- **Fix Required**: Debug and fix test00.asm program logic
- **Workaround**: Test is currently disabled due to program logic error
