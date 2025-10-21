ifndef TYPE
TYPE=debug
endif

# Auto-detect platform if not specified
ifndef PLATFORM
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
PLATFORM=linux
endif
ifeq ($(UNAME_S),Darwin)
PLATFORM=macos
endif
endif

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

ifeq ($(PLATFORM),linux)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG -DOFI_NOTRACE 
PROFILEFLAGS = $(NODEBUGFLAGS) -pg
CCFLAGS += -Wall -D_REENTRANT
# Auto-detect CPU architecture for Linux
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
CCFLAGS += -march=x86-64
else ifeq ($(UNAME_M),aarch64)
# ARM64 - no specific march flag needed
else ifeq ($(UNAME_M),arm64)
# ARM64 - no specific march flag needed
else ifeq ($(UNAME_M),i386)
CCFLAGS += -march=i386
else ifeq ($(UNAME_M),i686)
CCFLAGS += -march=i686
endif
else
ifeq ($(PLATFORM),macos)
CC=gcc
DEBUGFLAGS = -g3 -DDEBUG
NODEBUGFLAGS = -O3 -DNDEBUG
CCFLAGS += -Wall -D_REENTRANT
# Auto-detect CPU architecture for macOS
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
CCFLAGS += -march=x86-64
else ifeq ($(UNAME_M),arm64)
# ARM64 - no specific march flag needed
endif
else
$(error bad PLATFORM=$(PLATFORM). Supported platforms: linux, macos)
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
LIBRARY = $(LIBDIR)/lib$(LIBNAME).a
LIBRARIES += $(LIBRARY)
LINKLIBS += $(LIBNAME:%=-l%)
endif

ifdef SHAREDLIBNAME
ifeq ($(PLATFORM),macos)
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).dylib
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBNAME:%=-l%)
else
SHAREDLIBRARY = $(LIBDIR)/lib$(SHAREDLIBNAME).so
LIBRARIES += $(SHAREDLIBRARY)
LINKLIBS += $(SHAREDLIBNAME:%=-l%)
endif
endif

ifdef LIBNAMES
LIBRARIES += $(LIBNAMES:%=$(LIBDIR)/lib%.a)
LINKLIBS += $(LIBNAMES:%=-l%) 
endif

ifdef SHAREDLIBNAMES
LINKLIBS += $(SHAREDLIBNAMES:%=-l%) 
endif

ifdef BINNAMES
BINARIES = $(BINNAMES:%=$(BINDIR)/%)
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

$(OBJDIR)/%.o : %.cpp
	$(CC) $< -c $(CCFLAGS) -o $@

ifdef LIBNAME
$(LIBRARY) : $(OBJECTS)
	ar crsv $@ $(OBJECTS)
endif

ifdef SHAREDLIBNAME
ifeq ($(PLATFORM),macos)
$(SHAREDLIBRARY): $(OBJECTS)
	cc -o $@ -dynamiclib -mmacosx-version-min=10.9 $(OBJECTS)
else
$(SHAREDLIBRARY): $(OBJECTS)
	ld -o $@ -Bshareable $(OBJECTS)
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
