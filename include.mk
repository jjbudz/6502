ifndef TYPE
TYPE=debug
endif

#ifndef PLATFORM
#PLATFORM=win32_x86
#endif

ifeq ($(TYPE),debug)
else
ifeq ($(TYPE),release)
else
ifeq ($(TYPE),profile)
else
$(error bad TYPE=$(TYPE))
endif
endif
endif

ifeq ($(PLATFORM),win32_x86)
CC=cl.exe
DEBUGFLAGS = -Zi -GZ -Ge -DDEBUG -MTd -nologo 
NODEBUGFLAGS = -O2 -DNDEBUG -MT -nologo
PROFILEFLAGS = $(NODEBUGFLAGS) 
CCFLAGS += -GX -W3 -DWIN32 -GR -G6
else
ifeq ($(PLATFORM),redhat_x86-64)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG -DOFI_NOTRACE 
PROFILEFLAGS = $(NODEBUGFLAGS) -pg
CCFLAGS += -Wall -D_REENTRANT -march=x86-64
else
ifeq ($(PLATFORM),redhat_x86)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG -DOFI_NOTRACE 
PROFILEFLAGS = $(NODEBUGFLAGS) -pg
CCFLAGS += -Wall -D_REENTRANT -march=i386
else
ifeq ($(PLATFORM),cygwin_x86)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -D_REENTRANT
else
ifeq ($(PLATFORM),macos_x86)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -D_REENTRANT
else
ifeq ($(PLATFORM),macos_x86_64)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -Wall -D_REENTRANT -march=x86-64
else
ifeq ($(PLATFORM),macos_arm64)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -Wall -D_REENTRANT
else
ifeq ($(PLATFORM),ubuntu_x86-64)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG -DOFI_NOTRACE 
PROFILEFLAGS = $(NODEBUGFLAGS) -pg
CCFLAGS += -Wall -D_REENTRANT -march=x86-64
else
ifeq ($(PLATFORM),rpi_arm64)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -Wall -D_REENTRANT
else
$(error bad PLATFORM=$(PLATFORM))
endif
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(TYPE),debug)
CCFLAGS += $(DEBUGFLAGS)
else
ifeq ($(TYPE),release)
CCFLAGS += $(NODEBUGFLAGS)
else
ifeq ($(TYPE),profile)
CCFLAGS += $(PROFILEFLAGS)
else
$(error bad TYPE=$(TYPE))
endif
endif
endif

LIBDIR = lib/$(TYPE)/$(PLATFORM)
BINDIR = bin/$(TYPE)/$(PLATFORM)
OBJDIR = obj/$(TYPE)/$(PLATFORM)
OBJECTS = $(LIBSOURCE:%.cpp=$(OBJDIR)/%.o)
LINKLIBS =

ifdef LIBNAME
ifeq ($(PLATFORM),win32_x86)
LIBRARY = $(LIBDIR)/lib$(LIBNAME).lib
LIBRARIES += $(LIBRARY)
LINKLIBS += $(LIBRARY)
else
LIBRARY = $(LIBDIR)/lib$(LIBNAME).a
LIBRARIES += $(LIBRARY)
LINKLIBS += $(LIBNAME:%=-l%)
endif
endif

ifdef SHAREDLIBNAME
ifeq ($(PLATFORM),win32_x86)
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).lib
SHAREDLIBRARYDLL = $(LIBDIR)/lib$(SHAREDLIBNAME).dll
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBRARY)
else
ifeq ($(PLATFORM),cygwin_x86)
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).dll
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBNAME:%=-l%)
else
ifneq (,$(filter $(PLATFORM),macos_x86 macos_x86_64 macos_arm64))
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).dylib
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBNAME:%=-l%)
else
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).so
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBNAME:%=-l%)
endif
endif
endif
endif

ifdef LIBNAMES
ifeq ($(PLATFORM),win32_x86)
LIBRARIES += $(LIBNAMES:%=$(LIBDIR)/lib%.lib)
LINKLIBS += $(LIBRARIES)
else
LIBRARIES += $(LIBNAMES:%=$(LIBDIR)/lib%.a)
LINKLIBS += $(LIBNAMES:%=-l%) 
endif
endif

ifdef SHAREDLIBNAMES
ifeq ($(PLATFORM),win32_x86)
LINKLIBS += $(SHAREDLIBNAMES:%=$(LIBDIR)/lib%.lib) 
else
LINKLIBS += $(SHAREDLIBNAMES:%=-l%) 
endif
endif

ifdef BINNAMES
ifeq ($(PLATFORM),win32_x86)
BINARIES = $(BINNAMES:%=$(BINDIR)/%.exe)
else
ifeq ($(PLATFORM),cygwin_x86)
BINARIES = $(BINNAMES:%=$(BINDIR)/%.exe)
else
BINARIES = $(BINNAMES:%=$(BINDIR)/%)
endif
endif
endif

all: dirs lib bin

dirs: $(OBJDIR) $(LIBDIR) $(BINDIR)

lib: $(LIBRARY) $(SHAREDLIBRARY) $(SHAREDLIBRARYDLL)

test: $(TESTNAMES)

$(TESTNAMES) : 
	@export LD_LIBRARY_PATH=$(LIBDIR) ; \
	echo "----------------------------------------------------"; \
	echo "+ BEGIN TEST $@"; \
	echo "----------------------------------------------------"; \
	$(BINDIR)/$@ ; \
	echo "----------------------------------------------------"; \
	echo "+ END TEST $@"; \
	echo "----------------------------------------------------"

ifeq ($(PLATFORM),win32_x86)
$(OBJDIR)/%.o : %.cpp
	$(CC) $< -c $(CCFLAGS) -Fo$@
else
$(OBJDIR)/%.o : %.cpp
	$(CC) $< -c $(CCFLAGS) -o $@
endif

ifdef LIBNAME
ifeq ($(PLATFORM),win32_x86)
$(LIBRARY) : $(OBJECTS)
	lib /nologo /out:$@ $(OBJECTS)
else
$(LIBRARY) : $(OBJECTS)
	ar crsv $@ $(OBJECTS)
endif
endif

ifdef SHAREDLIBNAME
ifeq ($(PLATFORM),win32_x86)
$(SHAREDLIBRARY): $(OBJECTS)
	lib /nologo /out:$@ $(OBJECTS)

$(SHAREDLIBRARYDLL): $(OBJECTS)
	link /nologo /dll /out:$@ $(OBJECTS)
else
ifneq (,$(filter $(PLATFORM),macos_x86 macos_x86_64 macos_arm64))
$(SHAREDLIBRARY): $(OBJECTS)
	cc -o $@ -dynamiclib -mmacosx-version-min=10.9 $(OBJECTS)
#	cc -o $@ -dynamic -undefined dynamic_lookup -single_module -macosx_version_min 10.6 -lcrt1.10.6.o -lc -ldylib1.o -lSystem $(OBJECTS)
else
$(SHAREDLIBRARY): $(OBJECTS)
	ld -o $@ -Bshareable $(OBJECTS)
endif
endif
endif

bin: $(BINARIES)

$(OBJDIR) $(LIBDIR) $(BINDIR):
	if [ ! -e $@ ] ; then \
		mkdir -p $@ ; \
	fi

clean:
	$(RM) -f $(OBJECTS)
	$(RM) -f $(LIBRARY)
	$(RM) -f $(SHAREDLIBRARY) $(SHAREDLIBRARYDLL)
	$(RM) -f $(BINARIES)
