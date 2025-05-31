# $@ - target name
# $< - name the first target for list of dependencies
# $^ - the entire list of dependencies
# target and depens may be file names or phony targets
# target(s): dependencie(s)
#     command - action to produce the target
# make <default goal> or make (default goal will be first target from top to bottom)
# PHONY reports that target "clean" isn't a file
CC = gcc

ifeq ($(RELEASE), 1)
	CFLAGS = -Wall -I include -static -nostdlib -std=c99 -O2 -mavx2
else
	CFLAGS = -Wall -I include -static -nostdlib -std=c99 -O0 -g
endif

LIB_INCLUDE = $(wildcard include/*.h)
LIB_SRCMODS = $(wildcard src/*.c)
LIB_OBJMODS = $(patsubst %.c, %.o, $(LIB_SRCMODS))

TEST_SRCMODS_DIRS = tests
TEST_SRCMODS = $(wildcard $(TEST_SRCMODS_DIRS)/*.c)
TEST_BINS = $(patsubst %.c, %, $(TEST_SRCMODS))

.PHONY: all tests clean fmt checkvar 

all: libcfa.a tests

libcfa.a: $(LIB_OBJMODS)
	ar -rcs $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

tests: $(TEST_BINS)

%: $(TEST_SRCMOD_DIRS)%.c $(LIB_OBJMODS)
	$(CC) $(CFLAGS) $< $(LIB_OBJMODS) -o $@

clean:
	rm -f $(LIB_OBJMODS) $(TEST_BINS) libcfa.a

fmt:
	indent -kr -ts4 -l120 $(TEST_SRCMODS) $(LIB_SRCMODS) $(LIB_INCLUDE) && rm -f $(TEST_SRCMODS_DIRS)/*.c~ src/*.c~ include/*.h~

checkvar:
	@echo "LIB_SRCMODS = $(LIB_INCLUDE)"
	@echo "LIB_SRCMODS = $(LIB_SRCMODS)"
	@echo "LIB_OBJMODS = $(LIB_OBJMODS)"

	@echo "TEST_SRCMODS = $(TEST_SRCMODS)"
	@echo "TEST_BINS = $(TEST_BINS)"
