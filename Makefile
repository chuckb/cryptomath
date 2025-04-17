CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I/usr/local/opt/sqlite3/include
DEBUG_CFLAGS = -Wall -Wextra -ggdb -O0 -MMD -MP -I/usr/local/opt/sqlite3/include
# Works on macOS - Homebrew cask
# Note that native sqlite3 on macOS does not have the .load function enabled
LDFLAGS = -L/usr/local/opt/sqlite3/lib -lgmp -lsqlite3
# Works on Linux
#LDFLAGS = -I/usr/include/sqlite3 -L/usr/lib/x86_64-linux-gnu -lgmp

# Define the source files
SRCS = test_cryptomath.c crypto_get_types.c crypto_get_denoms.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
DEBUG_OBJS = $(SRCS:.c=.debug.o)

# Define the target executable
TARGET = test_cryptomath
SQLITE_EXT = crypto_decimal_extension.dylib

# Default target
all: $(TARGET) $(SQLITE_EXT)

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(TARGET)


# Build the SQLite extension
$(SQLITE_EXT): crypto_decimal_extension.c cryptomath.h crypto_get_types.o crypto_get_denoms.o
#	$(CC) -fPIC -shared $(CFLAGS) -o $@ $< $(LDFLAGS) -lsqlite3 -lm
	$(CC) -fPIC -dynamiclib $(CFLAGS) -o $@ crypto_decimal_extension.c crypto_get_types.o crypto_get_denoms.o $(LDFLAGS)

# Build the test executable
$(TARGET): $(OBJS) $(SQLITE_EXT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.c cryptomath.h
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	rm -rf $(OBJS) $(TARGET) $(DEPS) $(DEBUG_OBJS) $(SQLITE_EXT) $(SQLITE_EXT).dSYM crypto_decimal_extension.d

# Include auto-generated dependencies, but do not error if they don't exist yet:
-include $(DEPS)

# Run tests
test: $(TARGET)
	./$(TARGET)

.PHONY: all clean test debug