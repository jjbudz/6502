Building on macOS
-----------------

The build system automatically detects your CPU architecture (Intel x86_64 or Apple Silicon ARM64).

From a terminal:

```
  cd 6502
  make TYPE=release
  export PATH=$PATH:`pwd`/bin/release/macos
  cd test
  make test
  cd ..
```

Or to explicitly specify the platform:

```
  cd 6502
  make PLATFORM=macos TYPE=release
```

Building on Linux
-----------------

The build system automatically detects your CPU architecture (x86_64, ARM64, etc.).

From a terminal:

```
  cd 6502
  make TYPE=release
  export PATH=$PATH:`pwd`/bin/release/linux
  cd test
  make test
  cd ..
```

Or to explicitly specify the platform:

```
  cd 6502
  make PLATFORM=linux TYPE=release
```

Tests
-----

Unit tests can be found in the /test directory. Each unit test when run from
the batch script will test for an expected memory value in a given location.
Failed tests will show the expected and found values with a status of "false".

A test can be run from the command line within the test directory as follows:

```
  6502 -c test05.asm -r 4000 -a 0040:33
```

To run the test in trace mode, add the -t flag as follows:


```
  6502 -c test05.asm -r 4000 -a 0040:33 -t
```

To run the test in debug mode, change the -r flag to -d as follows:

```
  6502 -c test05.asm -d 4000 -a 0040:33 -t
```

Use the "help" command from the debugger to list commands.




