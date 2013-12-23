The 6502 program supports a barebone assembler, loader, and debugger. However, the
assembler's parser is a bit rough and requires different symbols to access each of
the various addressing modes. Use the "-i" flag to get a list of known instructions.

Hexidecimal address values must be prefixed with $, e.g.:

```
  STAA $8000
  STAZ $80
```

Immediate values are denoted using #, e.g.:

```asm
  LDAI #59
  LDAI #$5A
```

Jumps and branch destinations may be specified using labels as follows:

```asm
  foo JMP bar
      LDAI #00
      STAA $8000
      BRK
  bar LDAI #01
      STAA $8000
      BRK
```

Sample syntax for the assembler can be found by looking at the .asm files
in the [tests/](tests) directory.

Program command line arguments include:

```
  -l <filename> -a <filename> -s <filename> -r [<address>] [-t] [-p] where:
  -h to display command line options
  -l <filename> to load an object file
  -c <filename> to compile source file
  -s <filename> to save object file after assembly
  -r <address> to run code from the address (hexadecimal, e.g. A000)
  -d <address> to debug code from the address (hexadecimal, e.g. A000)
  -a <address>:<value> to assert value matches at the given address
  -t to turn on trace output
  -i to list assembler instructions
  -p[rfsm] to print (dump) registers, flags, stack, and memory on exit
  -v to print version information
```

Command line examples:

```bash
  6502 -v
  6502 -i
  6502 -c LDAI2.asm -r 4000 -a 8000:7f -pr
  6502 -c LDAI2.asm -d 4000
  6502 -c LDAI2.asm -s LDAI2.6502
  6502 -l LDAI2.6502 -r 4000 -a 8000:7f -psfr
```
  
  
  

