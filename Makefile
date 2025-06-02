# $@ - target name
# $< - name the first target for list of dependencies
# $^ - the entire list of dependencies
# target and depens may be file names or phony targets
# target(s): dependencie(s)
#     command - action to produce the target
# make <default goal> or make (default goal will be first target from top to bottom)
# PHONY reports that target "clean" isn't a file
CC = gcc
LIBINCLUDE = -I lib/cfa/include

ifeq ($(RELEASE), 1)
	CFLAGS = -Wall $(LIBINCLUDE) -static -nostdlib -std=c99 -O2 -mavx2
else
	CFLAGS = -Wall $(LIBINCLUDE) -static -nostdlib -std=c99 -O0 -ggdb -DDEBUG_PRINT
endif

PROGRAM_NAME = bin/chat_server

LIBS = -l cfa -L lib/cfa
LIBSDEPS = lib/cfa/libcfa.a

INCLUDE = $(wildcard src/*.h)
SRCMODS = $(wildcard src/*.c)
OBJMODS = $(patsubst %.c, %.o, $(SRCMODS))

TEST_SRCMODS_DIRS = tests
TEST_SRCMODS = $(wildcard $(TEST_SRCMODS_DIRS)/*.c)
TEST_BINS = $(patsubst %.c, %, $(TEST_SRCMODS))

# -* - disable all default checks, enable all clang-analyser-* checks
# except c++ ones (-clang-analyzer-cplusplus*), enable perfomance checks.
# -header-filter - what header files to analyze.
TIDYFLAGS = -warnings-as-errors=CHECKS='-*,clang-analyzer-*,-clang-analyzer-cplusplus*,performance*' --system-headers

.PHONY: all tests clean fmt macro disas prof vettest vet

all: $(PROGRAM_NAME) tests

$(PROGRAM_NAME): $(OBJMODS) $(LIBSDEPS)
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

lib/cfa/libcfa.a:
ifeq ($(RELEASE), 1)
		cd lib/cfa/ ; $(MAKE) RELEASE=1
else
		cd lib/cfa/ ; $(MAKE) 
endif

tests: $(TEST_BINS)

macro: $(OBJMODS) $(LIBSDEPS)
	$(CC) -E $(LIBINCLUDE) $(SRCMODS)

disas: $(PROGRAM_NAME)
	objdump -M intel -d $< > $<.s

prof: $(OBJMODS) $(LIBSDEPS)
	$(CC) $(CFLAGS) -fno-omit-frame-pointer $< $(LIBS) -o $(PROGRAM_NAME)

vet:
	clang-tidy $(TIDYFLAGS) src/*.c -- $(CFLAGS) -O2 -mavx2

vettest:
	clang-tidy $(TIDYFLAGS) tests/*.c -- $(CFLAGS) -O2 -mavx2

%: $(TEST_SRCMOD_DIRS)%.c $(OBJMODS)
	$(CC) $(CFLAGS) $< $(OBJMODS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJMODS) $(TEST_BINS) $(PROGRAM_NAME) $(PROGRAM_NAME).s ; cd lib/cfa && $(MAKE) clean

fmt:
	indent -kr -ts4 -l120 $(TEST_SRCMODS) $(SRCMODS) && rm -f $(TEST_SRCMODS_DIRS)/*.c~ src/*.c~ include/*.h~
