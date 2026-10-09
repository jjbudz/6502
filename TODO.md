A partial list of things that don't work or are in need of enhancement:

1. Unit tests - a bunch have been generated but more complex tests are needed.
1. The assembler ([docs/ASSEMBLER_PLAN.md](docs/ASSEMBLER_PLAN.md)) has no macros,
   includes, conditional assembly or local labels, and forward references are always
   encoded as absolute addresses.
1. Various @todos need to be addressed.
