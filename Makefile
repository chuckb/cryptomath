CC = gcc
CFLAGS = -Wall -Wextra -g -O2
DEBUG_CFLAGS = -Wall -Wextra -ggdb -O0 -MMD -MP
LDFLAGS = -lgmp

# Define the source files
SRCS = test_cryptomath2.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
DEBUG_OBJS = $(SRCS:.c=.debug.o)

# Define the target executable
TARGET = test_cryptomath2

# Default target
all: $(TARGET)

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(TARGET)

# Build the test executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.c cryptomath2.h
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	rm -f $(OBJS) $(TARGET) $(DEPS) $(DEBUG_OBJS)

# Include auto-generated dependencies, but do not error if they don't exist yet:
-include $(DEPS)

# Run tests
test: $(TARGET)
	./$(TARGET)

.PHONY: all clean test debug