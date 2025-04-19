# Common compiler settings
CC = gcc
# -O2 is messing with SHOULD_ASSERT logic in test harness under Linux.
# I have not bothered to figure out why yet.
CFLAGS = -Wall -Wextra -O0 -MMD -MP
DEBUG_CFLAGS = -Wall -Wextra -ggdb -O0 -MMD -MP

#── 1) detect platform ────────────────────────────────────────────────────────────
UNAME_S := $(shell uname -s)

#── 2) on macOS, if brew is around, prepend its pkgconfig dirs ───────────────────
ifeq ($(UNAME_S),Darwin)
	EXTENSION_SUFFIX = dylib
	EXTENSION_FLAGS = -dynamiclib
	ifneq (,$(shell command -v brew 2>/dev/null))
    	# For sqlite3:
	    ifneq (,$(shell brew --prefix sqlite3 2>/dev/null))
			PKG_CONFIG_PATH := $(shell brew --prefix sqlite3)/lib/pkgconfig:$(PKG_CONFIG_PATH)
    	endif
		# For gmp:
		ifneq (,$(shell brew --prefix gmp 2>/dev/null))
			PKG_CONFIG_PATH := $(shell brew --prefix gmp)/lib/pkgconfig:$(PKG_CONFIG_PATH)
		endif
	endif
else
	EXTENSION_SUFFIX = so
	EXTENSION_FLAGS = -shared
endif

# Default settings
SQLITE_INCLUDE_DIR ?= /usr/include
SQLITE_LIB_DIR ?= /usr/lib
GMP_INCLUDE_DIR ?= /usr/include
GMP_LIB_DIR ?= /usr/lib

#── 3) set up pkg-config ───────────────────────────────────────────────────────────
ifneq (,$(shell command -v pkg-config 2>/dev/null))
	ifeq ($(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --atleast-version=3.44.0 sqlite3 && echo ok),ok)
		SQLITE_INCLUDE_DIR := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --variable=includedir sqlite3)
		SQLITE_LIB_DIR := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --variable=libdir sqlite3)
		GMP_INCLUDE_DIR := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --variable=includedir gmp)
		GMP_LIB_DIR := $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --variable=libdir gmp)
	else
		$(error sqlite3 version 3.44.0 or later is required; you have $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --modversion sqlite3))
	endif
endif

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
INCLUDE_FLAGS = -I$(INCLUDE_DIR) -I$(SQLITE_INCLUDE_DIR) -I$(GMP_INCLUDE_DIR)
LDFLAGS = -L$(SQLITE_LIB_DIR) -L$(GMP_LIB_DIR) -lgmp -lsqlite3

# Common targets
.PHONY: all clean test debug

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
