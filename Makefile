# compiler, linker, archiver and flags
CC=gcc
LD=gcc
AR=ar
CFLAGS= -c -fPIC -O2 -Wall -pedantic -I include
LDFLAGS=

# directories
LIB_DIR=lib
SRC_DIR=src
INCLUDE_DIR=include
TEST_DIR=test
BUILD_DIR=build
# files
LIB_SRC=error.c sigil.c
LIB_O=$(LIB_SRC:%.c=$(BUILD_DIR)/$(LIB_DIR)/%.o)
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

# lib *.c -> *.o
$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)/$(LIB_DIR)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: test
test: $(BUILD_DIR)/libpdf-sigil.a $(TEST_O) | $(BUILD_DIR)/$(TEST_DIR)
	$(LD) $(LDFLAGS) $(TEST_O) $(BUILD_DIR)/libpdf-sigil.a -o $(BUILD_DIR)/$(TEST_DIR)/test
	$(BUILD_DIR)/$(TEST_DIR)/test

# test *.c -> *.o
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)/$(TEST_DIR)
	$(CC) $(CFLAGS) $< -o $@

# If BUILD_DIR does not exist, create it
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# If BUILD_DIR/LIB_DIR does not exist, create it
$(BUILD_DIR)/$(LIB_DIR):
	mkdir -p $(BUILD_DIR)/$(LIB_DIR)

# If BUILD_DIR/TEST_DIR does not exist, create it
$(BUILD_DIR)/$(TEST_DIR):
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)

# Set proper dependencies of c files on headers
$(LIB_DIR)/error.c: $(INCLUDE_DIR)/error.h
$(LIB_DIR)/sigil.c: $(INCLUDE_DIR)/sigil.h
$(TEST_DIR)/test.c: $(INCLUDE_DIR)/error.h $(INCLUDE_DIR)/sigil.h

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
