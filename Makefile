.DEFAULT_GOAL := all

# Include common definitions
include include.mk

# Include component-specific makefiles
include lib.mk
include sqlite.mk

# Test executable settings
LIB_TEST_SRCS = $(TEST_DIR)/test_lib.c
LIB_TEST_TARGET = $(BUILD_DIR)/test_lib
LIB_TEST_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(LIB_TEST_SRCS:.c=.o)))
LIB_TEST_DEPS = $(LIB_TEST_OBJS:.o=.d)

SQLITE_TEST_SRCS = $(TEST_DIR)/test_sqlite.c
SQLITE_TEST_TARGET = $(BUILD_DIR)/test_sqlite
SQLITE_TEST_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SQLITE_TEST_SRCS:.c=.o)))
SQLITE_TEST_DEPS = $(SQLITE_TEST_OBJS:.o=.d)

# Default target
all: $(LIB_TEST_TARGET) $(SQLITE_TEST_TARGET) $(SQLITE_EXT)

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(LIB_TEST_TARGET) $(SQLITE_TEST_TARGET) $(SQLITE_EXT)

# Build library test executable
$(LIB_TEST_TARGET): $(LIB_TEST_OBJS) $(LIB_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(LIB_TEST_OBJS) $(LDFLAGS)

# Build SQLite test executable
$(SQLITE_TEST_TARGET): $(SQLITE_TEST_OBJS) $(SQLITE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(SQLITE_TEST_OBJS) $(LDFLAGS)

# Compile test files
$(LIB_TEST_OBJS): $(BUILD_DIR)/%.o: $(TEST_DIR)/%.c $(LIB_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

$(SQLITE_TEST_OBJS): $(BUILD_DIR)/%.o: $(TEST_DIR)/%.c $(SQLITE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Clean everything
clean: clean-lib clean-sqlite
	rm -f $(LIB_TEST_OBJS) $(LIB_TEST_DEPS) $(LIB_TEST_TARGET)
	rm -f $(SQLITE_TEST_OBJS) $(SQLITE_TEST_DEPS) $(SQLITE_TEST_TARGET)
	rm -rf $(BUILD_DIR)

# Run tests
test: $(LIB_TEST_TARGET) $(SQLITE_TEST_TARGET) $(SQLITE_EXT)
	./$(LIB_TEST_TARGET)
	./$(SQLITE_TEST_TARGET)

# Include auto-generated dependencies
-include $(LIB_TEST_DEPS) $(SQLITE_TEST_DEPS)