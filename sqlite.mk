# SQLite extension specific settings
SQLITE_EXT = crypto_decimal_extension.dylib
SQLITE_SRCS = $(SRC_DIR)/crypto_decimal_extension.c $(SRC_DIR)/crypto_get_types.c $(SRC_DIR)/crypto_get_denoms.c
SQLITE_OBJS = $(SQLITE_SRCS:.c=.o)
SQLITE_DEPS = $(SQLITE_SRCS:.c=.d)

# Header dependencies
SQLITE_HEADERS = $(INCLUDE_DIR)/cryptomath.h $(INCLUDE_DIR)/cypto_get_types.h $(INCLUDE_DIR)/cypto_get_denoms.h

# Build the SQLite extension
$(SQLITE_EXT): $(SQLITE_OBJS) $(SQLITE_HEADERS)
	$(CC) -fPIC -dynamiclib $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(SQLITE_OBJS) $(LDFLAGS)

# Compile SQLite extension objects
$(SQLITE_OBJS): %.o: %.c $(SQLITE_HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Clean SQLite extension
clean-sqlite:
	rm -f $(SQLITE_EXT) $(SQLITE_EXT).dSYM $(SQLITE_OBJS) $(SQLITE_DEPS)

.PHONY: clean-sqlite 