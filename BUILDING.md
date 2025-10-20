Mac
---

From a terminal:

**For Intel-based Macs (x86_64):**

```
  cd 6502
  make PLATFORM=macos_x86_64 TYPE=release
  export PATH=$PATH:`pwd`/bin/release/macos_x86_64
  cd test
  sh unittest.script
  cd ..
```

**For Apple Silicon Macs (ARM64/M1/M2/M3):**

```
  cd 6502
  make PLATFORM=macos_arm64 TYPE=release
  export PATH=$PATH:`pwd`/bin/release/macos_arm64
  cd test
  sh unittest.script
  cd ..
```

**For older Intel-based Macs (legacy x86):**

```
  cd 6502
  make PLATFORM=macos_x86 TYPE=release
  export PATH=$PATH:`pwd`/bin/release/macos_x86
  cd test
  sh unittest.script
  cd ..
```

Raspberry Pi (Bookworm, 64-bit)
--------------------------------

From a terminal:

```
  cd 6502
  make PLATFORM=rpi_arm64 TYPE=release
  export PATH=$PATH:`pwd`/bin/release/rpi_arm64
  cd test
  sh unittest.script
  cd ..
```

Windows
-------

Use Microsoft Visual 2008 to load the solution file and build the release target.

After succesfully building, start a command prompt and use the following:

```
  cd 6502
  set PATH=%PATH%;%CD%\Release\
  cd test
  unittest
  cd ..   
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




