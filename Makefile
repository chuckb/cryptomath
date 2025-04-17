.DEFAULT_GOAL := all

# Include common definitions
include include.mk

# Include component-specific makefiles
include lib.mk
include sqlite.mk

# Test executable settings
TEST_SRCS = $(TEST_DIR)/test_cryptomath.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_DEPS = $(TEST_SRCS:.c=.d)
TEST_TARGET = test_cryptomath

# Header dependencies
TEST_HEADERS = $(LIB_HEADERS) $(SQLITE_HEADERS)

# Default target
all: $(TEST_TARGET) $(SQLITE_EXT)

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(TEST_TARGET) $(SQLITE_EXT)

# Build test executable
$(TEST_TARGET): $(TEST_OBJS) $(SQLITE_EXT)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(TEST_OBJS) $(LDFLAGS)

# Compile test files
$(TEST_OBJS): %.o: %.c $(TEST_HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Clean everything
clean: clean-lib clean-sqlite
	rm -f $(TEST_OBJS) $(TEST_DEPS) $(TEST_TARGET)

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Include auto-generated dependencies
-include $(TEST_DEPS) $(SQLITE_DEPS)