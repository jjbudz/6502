A partial list of things that don't work or are in need of enhancement:

1. Unit tests - a bunch have been generated but more complex tests are needed.
1. Some non-nominal tests of the assembler would be useful.
1. The assembler/parser code is in bad need of refactoring. The plan is in
   [docs/ASSEMBLER_PLAN.md](docs/ASSEMBLER_PLAN.md); items 2, 4 and 5 here are folded into it.
1. The assembler should accept a literal address as a branch operand (e.g. `BEQ $400C`).
   Today only symbolic labels work: a literal is assembled as a 16-bit absolute operand,
   while the CPU reads branches as an 8-bit relative offset, so the branch lands in the
   wrong place (usually an unimplemented opcode). The assembler should compute the
   relative offset from the literal, and reject targets outside the -128..+127 range.
   Fold this into the assembler rework above. See test/README.md "Test File Format".
1. The assembler should be enhanced to not require explicit addressing/indexing modes.
1. Various @todos need to be addressed.
