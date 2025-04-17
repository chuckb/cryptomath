# Common compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -O2 -MMD -MP
DEBUG_CFLAGS = -Wall -Wextra -ggdb -O0 -MMD -MP

# Common directories
INCLUDE_DIR = include
SRC_DIR = src
TEST_DIR = tests

# Common flags
INCLUDE_FLAGS = -I$(INCLUDE_DIR) -I/usr/local/opt/sqlite3/include
LDFLAGS = -L/usr/local/opt/sqlite3/lib -lgmp -lsqlite3

# Common targets
.PHONY: all clean test debug 