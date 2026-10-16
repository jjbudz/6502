# Assembler Rework Plan

**Status: complete.** All five phases below are done (PRs #55 to #59). The
assembler now lives in `asm6502.cpp`/`asm6502.h` as a two-pass lexer and
parser; the comment at the top of `asm6502.cpp` describes how it works.
`assemble()` in `l6502.cpp` is a thin wrapper around it. What the assembler
still does not support is listed under [Not yet supported](#not-yet-supported).

This document is kept as a record of why the rework was done and the
decisions made along the way.

## Problems with the original assembler

The original assembler in `l6502.cpp` (`assemble()`, `resolve()`,
`getToken()`, and the `labels`/`branches` maps) was a single-pass token loop
that classified each token by its first character and its position on the
line. It had these problems, all fixed by the rework:

- **Mnemonic typos were accepted.** Any token the assembler did not recognize
  was assumed to be a branch/jump label. Space was reserved based on whatever
  byte preceded it, and assembly reported success.
- **Label definitions were detected by prefix match.** A token was treated as
  a label definition when the line *started with* that token, so in
  `LOOP2  BNE LOOP` the operand `LOOP` matched the start of `LOOP2` and was
  redefined.
- **Errors were swallowed or fatal.** `resolve()`'s result was ignored, so an
  unresolved label still returned success. `findLabel()` returned `0` for
  "not found", so a label at `$0000` looked missing. `calcOffset()` called
  `exit()` on an out-of-range branch. `errno` was not cleared before
  `strtol()`.
- **Branches only accepted labels.** A literal operand such as `BEQ $400C`
  was emitted as a 16-bit absolute operand even though the CPU reads an
  8-bit relative offset.
- **Addressing mode was spelled in the mnemonic.** `LDXZ`, `ADCIX`, etc. The
  standard `LDA ($10),Y` form was not understood.
- **It lived in a 3,500-line file** alongside the CPU and debugger.

## Goals

1. Accept standard 6502 syntax (`LDA #$10`, `LDA $1234,X`, `LDA ($10),Y`,
   `JMP ($1234)`) alongside the existing suffixed mnemonics. Every existing
   `test/*.asm` and `sample*.asm` must assemble to **byte-identical** output.
2. Report errors with line and column, collect all of them, and fail with a
   nonzero exit code. No `exit()` inside the library; no silent guesses.
3. Leave room for expressions, directives, and a listing output without
   another rewrite.

## Target structure

The assembler moves to its own translation unit, `asm6502.cpp`/`asm6502.h`.
`assemble(filename)` in `l6502.h` stays as a thin wrapper so `main.cpp` does
not change.

1. **Lexer.** Turns a line into typed tokens: numbers (`$hex`, decimal,
   `%binary`, `'c'`), identifiers, punctuation (`# ( ) , + - < > : = *`),
   strings, and `;` comments (to end of line, with or without a preceding
   space).
2. **Line parser.** `[label[:]] [mnemonic | directive] [operand] [; comment]`.
   A label is an identifier in column 0 or one ending in `:`. A leading
   `$xxxx` still means "set the location counter", so existing files work.
3. **Operand parser.** Infers the addressing mode from the operand's shape:
   `#e` immediate; `e`, `e,X`, `e,Y` direct; `(e,X)`, `(e),Y`, `(e)`
   indirect; none implied; `A` accumulator.
4. **Opcode table.** Maps (mnemonic, mode) to an opcode, generated from the
   existing `i6502` table. A small alias map translates legacy names
   (`LDXZ` → `LDX` zero page, `ADCIX` → `ADC (zp,X)`, …) so the irregular
   cases (`BIT`, `JMP`, `ASL`, `LDXY`) are handled in one place.

### Two passes

- **Pass 1** sizes each instruction, assigns label addresses, and handles
  `.ORG` and `NAME = value`.
- **Pass 2** evaluates operands and emits bytes.

This replaces the `branches` map and `resolve()`. Branches always take an
address and convert it to a relative offset, which fixes `BEQ $400C`. An
out-of-range offset is a reported error, not an `exit()`.

**Zero page vs. absolute** is decided as follows:

- A value under `$100` that is known in pass 1 is zero page.
- A 4-digit literal (`$0050`) stays absolute, as today.
- A label defined later in the file defaults to absolute.

The last rule is the usual compromise; it keeps both passes in agreement
about instruction sizes.

## Phases

Each phase was one pull request; all five are done. Phase 5 also added
`.EQU`, strings in `.BYTE`, and running from the reset vector (`-r` with no
address), after a look at what other 6502 assemblers do.

| # | PR | Done when |
|---|----|-----------|
| 1 (#55) | **Test safety net.** Record the assembled bytes of every `test/*.asm` and `sample*.asm` as golden files. Add a way to run "this source must fail to assemble" tests. Make sure `-c` failures exit nonzero. | Golden check passes on current `master`; `make test` runs it |
| 2 (#56) | **Restructure, no behaviour change.** Add `asm6502.cpp`, lexer, line parser, two passes. Legacy syntax only. Delete `branches`, `resolve()` and the label helpers (`getToken` stays: the debugger uses it). Typos, duplicate labels and operands of the wrong width become errors. | All tests and all golden files match byte for byte |
| 3 (#57) | **Standard syntax.** Infer addressing modes; accept `LDA`, `STA`, etc. Branches accept literal addresses. | Tests for every addressing-mode shape, including rejected ones (`LDX ($10),X`) |
| 4 (#58) | **Expressions and directives.** `+ -`, `<`/`>` (low/high byte), `*` (location counter); `.ORG`, `.BYTE`, `.WORD`, `.TEXT`, `NAME = value`. | Tests for each, plus failure tests for undefined symbols and out-of-range values |
| 5 (#59) | **Polish.** `-L` listing output (address, bytes, source). Port `sample*.asm` to standard syntax. `.EQU`, strings in `.BYTE`, `-r` with no address runs from the reset vector at `$FFFC`. Update `DEVELOPMENT.md` and `CHEATSHEET.md`. | Docs describe both syntaxes; `LISTING` test pins the listing format |

### Phase 1 design

- `test/golden/<name>.hex` holds `xxd -a` output of the 64K image produced by
  `6502 -c <name>.asm -s <tmp>`. Autoskip (`-a`) collapses zero runs, so each
  golden file is a few lines.
- `test/golden.sh check` regenerates and diffs every golden file;
  `test/golden.sh update` rewrites them. `make golden` and
  `make golden-update` wrap these; `make test` runs the check.
- `test/fail/<name>.asm` must fail to assemble. An optional
  `test/fail/<name>.err` holds a substring that must appear in the assembler's
  output. `test/fail.sh` runs them; `make test` includes it.
- `assemble()` propagates `resolve()`'s return value, and `main.cpp` no
  longer lets a passing `-a` assertion mask an assembly failure. These are
  the only behaviour changes in phase 1.
- Since #67, a source file with assembler errors exits with status 3 (see
  the exit status table in `CHEATSHEET.md`), and `test/fail.sh` requires
  that status, so a crash or usage error does not count as an expected
  assembler failure.

## Decisions

- **Legacy mnemonics stay supported.** The alias map makes this cheap, and
  the test suite depends on them.
- **Labels and mnemonics are case-insensitive**, as today. Only string
  contents (`.TEXT "Hello"`) keep their case.

## Not yet supported

The rework met its goals; these were out of its scope and remain open (also
listed in `TODO.md`):

- **Conditional assembly** (`.IF`/`.ELSE`/`.ENDIF`). The condition would have
  to be known in pass 1, as constants already must be, because skipping a
  block moves every later address. It also needs comparison operators in
  expressions (see below).
- **Expression operators.** Expressions support `+`, `-`, unary `-`,
  `<`/`>` (low/high byte) and `*` (location counter). There are no
  comparisons (`=`, `<>`), no `*`, `/` and no bitwise operators. As binary
  operators `<` and `>` would clash with the low/high byte prefixes.
- **Macros, includes and local labels.**
- **Reserving space** (`.DS`/`.RES`); a variable needs `NAME = address`.
- **Zero page forward references.** A label defined later in the file is
  always encoded as absolute, even if it turns out to be in zero page (the
  compromise described under *Two passes*).

