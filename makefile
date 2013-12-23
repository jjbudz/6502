
# TODO: move bin stuff to include.mak and hide cross platform crap
#       define new SYSTEMLIBS and use them for each binary
#       would still need to ifdef SYSTEMLIBS for different platforms

LIBSOURCE = l6502.cpp ftrace.cpp ticker.cpp util.cpp

ifeq ($(PLATFORM),win32_x86)
LIBSOURCE += getopt.cpp
endif

LIBNAME = 6502
LIBNAMES =
SHAREDLIBNAMES =
BINNAMES = 6502
TESTNAMES =

CCFLAGS = -I.

include include.mk

ifeq ($(PLATFORM),win32_x86)

ifeq ($(TYPE),debug)
SYSLIBS=wsock32.lib sxlmtd.lib /nodefaultlib:libcmt.lib
else
ifeq ($(TYPE),profile)
SYSLIBS=wsock32.lib sxlmt.lib /profile
else
SYSLIBS=wsock32.lib sxlmt.lib
endif
endif

$(BINDIR)/6502.exe: $(LIBRARIES) main.cpp
	$(CC) main.cpp -Fe$@ -Fo$(OBJDIR)/main.obj $(CCFLAGS) -I. /link $(LINKLIBS) $(SYSLIBS)

else
ifeq ($(PLATFORM),cygwinx86)
$(BINDIR)/6502.exe: $(LIBRARIES) main.cpp
	$(CC) main.cpp -o $@ $(CCFLAGS) -I. -L$(LIBDIR) $(LINKLIBS) -lstdc++
else
$(BINDIR)/6502: $(LIBRARIES) main.cpp
	$(CC) main.cpp -o $@ $(CCFLAGS) -I. -L$(LIBDIR) $(LINKLIBS) -lstdc++
endif
endif
