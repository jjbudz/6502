
LIBSOURCE = l6502.cpp ftrace.cpp ticker.cpp util.cpp

LIBNAME = 6502
LIBNAMES =
SHAREDLIBNAMES =
BINNAMES = 6502
TESTNAMES =

CCFLAGS = -I.

include include.mk

$(BINDIR)/6502: $(LIBRARIES) main.cpp
	$(CC) main.cpp -o $@ $(CCFLAGS) -I. -L$(LIBDIR) $(LINKLIBS) -lstdc++

# Override the test target from include.mk to invoke the test directory makefile
.PHONY: test
test:
	$(MAKE) -C test test PLATFORM=$(PLATFORM) TYPE=$(TYPE)
