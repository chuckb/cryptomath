CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = -lgmp

# Define the source files
SRCS = test_cryptomath.c
OBJS = $(SRCS:.c=.o)

# Define the target executable
TARGET = test_cryptomath

# Default target
all: $(TARGET)

# Build the test executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.c cryptomath.h
	$(CC) $(CFLAGS) -c $<

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Run tests
test: $(TARGET)
	./$(TARGET)

.PHONY: all clean test 