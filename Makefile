# compiler, linker, archiver and flags
CC=gcc
LD=gcc
AR=ar
CFLAGS= -c -fPIC -Wall -pedantic -I include
LDFLAGS=

# debug
CFLAGS += -g -O0

# directories
LIB_DIR=lib
SRC_DIR=src
INCLUDE_DIR=include
TEST_DIR=test
BUILD_DIR=build
# library files
LIB_SRC=auxiliary.c config.c header.c sigil.c trailer.c xref.c
LIB_H=$(LIB_SRC:%.c=%.h) types.h constants.h
LIB_O=$(LIB_SRC:%.c=$(BUILD_DIR)/$(LIB_DIR)/%.o)
# test files
TEST_SRC=test.c
TEST_O=$(TEST_SRC:%.c=$(BUILD_DIR)/$(TEST_DIR)/%.o)

all: lib test

# static + shared library
lib: $(BUILD_DIR)/libpdf-sigil.a $(BUILD_DIR)/libpdf-sigil.so

# static library
$(BUILD_DIR)/libpdf-sigil.a: $(LIB_O) | $(BUILD_DIR)
	$(AR) rcs $@ $^

# shared library
$(BUILD_DIR)/libpdf-sigil.so: $(LIB_O) | $(BUILD_DIR)
	$(CC) -shared -fPIC -o $@ $^

# lib/*.c -> build/lib/*.o
$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)/$(LIB_DIR)
	$(CC) $(CFLAGS) $< -o $@

# run tests
.PHONY: test
test: $(BUILD_DIR)/$(TEST_DIR)/test
	$(BUILD_DIR)/$(TEST_DIR)/test

.PHONY: test_verbose
test_verbose: $(BUILD_DIR)/$(TEST_DIR)/test
	$(BUILD_DIR)/$(TEST_DIR)/test --verbose

.PHONY: test_quiet
test_quiet: $(BUILD_DIR)/$(TEST_DIR)/test
	$(BUILD_DIR)/$(TEST_DIR)/test --quiet

# test binary
$(BUILD_DIR)/$(TEST_DIR)/test: $(BUILD_DIR)/libpdf-sigil.a                     \
                               $(TEST_O) | $(BUILD_DIR)/$(TEST_DIR)
	$(LD) $(LDFLAGS) $(TEST_O) $(BUILD_DIR)/libpdf-sigil.a                     \
		-o $(BUILD_DIR)/$(TEST_DIR)/test

# test/*.c -> build/test/*.o
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)/$(TEST_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Dynamically create directory structure when needed
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(LIB_DIR):
	mkdir -p $(BUILD_DIR)/$(LIB_DIR)

$(BUILD_DIR)/$(TEST_DIR):
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f Makefile.deps

# Dependencies
include Makefile.deps

Makefile.deps: $(addprefix $(LIB_DIR)/,     $(LIB_SRC) )                       \
               $(addprefix $(INCLUDE_DIR)/, $(LIB_H)   )                       \
               $(addprefix $(TEST_DIR)/,    $(TEST_SRC))
	$(CC) -MM $(LIB_DIR)/*.c $(TEST_DIR)/*.c -I include |                      \
	sed "s@\(^.*\.o:.*\)@$(BUILD_DIR)/$(LIB_DIR)/\1@" > Makefile.deps
