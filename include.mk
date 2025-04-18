# Common compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -O2 -MMD -MP
DEBUG_CFLAGS = -Wall -Wextra -ggdb -O0 -MMD -MP

# Common directories
INCLUDE_DIR = include
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
DIST_DIR = dist

# Distribution settings
PACKAGE_NAME = cryptomath
VERSION = 1.0.0
DIST_PACKAGE = $(PACKAGE_NAME)-$(VERSION)
DIST_TARBALL = $(DIST_PACKAGE).tar.gz

# Common flags
INCLUDE_FLAGS = -I$(INCLUDE_DIR) -I/usr/local/opt/sqlite3/include
LDFLAGS = -L/usr/local/opt/sqlite3/lib -lgmp -lsqlite3

# Common targets
.PHONY: all clean test debug dist

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create dist directory
$(DIST_DIR):
	mkdir -p $(DIST_DIR) 