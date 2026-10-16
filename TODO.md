A partial list of things that don't work or are in need of enhancement:

1. **Assembler** (see [docs/ASSEMBLER_PLAN.md](docs/ASSEMBLER_PLAN.md#not-yet-supported)):
   no macros, includes, conditional assembly (`.IF`) or local labels;
   expressions support only `+`, `-`, `<`/`>` (low/high byte) and `*`, with no
   comparison, multiplication or bitwise operators; no directive to reserve
   space (`.DS`/`.RES`); and a label defined later in the file is always
   encoded as an absolute address, even when it turns out to be in zero page.
1. **Memory-mapped I/O.** Instructions read and write memory directly, so there
   is no bus layer for devices to sit on. Without it there is no character
   I/O, IRQ sources cannot be acknowledged by a handler (the `--irq-every`,
   `--nmi-every` and debugger sources release the line when the interrupt is
   taken), and Klaus Dormann's interrupt test, which raises interrupts by
   writing to a port, cannot run.
1. **NMOS interrupt timing quirks** are not emulated: `CLI`/`SEI`/`PLP` taking
   effect one instruction late, a taken branch delaying an interrupt, and an
   NMI hijacking `BRK`.
1. **Undocumented opcodes** are not implemented. Executing one, such as the
   65C02-only `$DB`, trips an `assert` in `step()` and aborts instead of
   reporting an error.
1. **Halting.** A run ends at `BRK` with no handler or at an instruction that
   jumps to itself. Longer infinite loops (`loop NOP` / `JMP loop`) run until
   interrupted.
1. **Tests.** Klaus Dormann's 6502 functional test and Bruce Clark's decimal
   mode test both pass (checked by hand; neither is in the repository). The
   suite has not been run on Linux on ARM, where `char` is unsigned.
1. Various `@todo`s need to be addressed (mostly in `l6502.h`).
