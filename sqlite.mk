# SQLite extension specific settings
SQLITE_EXT = $(BUILD_DIR)/crypto_decimal_extension.dylib
SQLITE_SRCS = $(SRC_DIR)/crypto_decimal_extension.c $(SRC_DIR)/crypto_get_types.c $(SRC_DIR)/crypto_get_denoms.c
SQLITE_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SQLITE_SRCS:.c=.o)))
SQLITE_DEPS = $(SQLITE_OBJS:.o=.d)

# Header dependencies
SQLITE_HEADERS = $(INCLUDE_DIR)/cryptomath.h $(INCLUDE_DIR)/cypto_get_types.h $(INCLUDE_DIR)/cypto_get_denoms.h

# Distribution files
DIST_SQLITE_EXT = $(DIST_DIR)/$(DIST_PACKAGE)/lib/$(notdir $(SQLITE_EXT))

# Build the SQLite extension
$(SQLITE_EXT): $(SQLITE_OBJS) $(SQLITE_HEADERS) | $(BUILD_DIR)
	$(CC) -fPIC -dynamiclib $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(SQLITE_OBJS) $(LDFLAGS)

# Compile SQLite extension objects
$(SQLITE_OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SQLITE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Clean SQLite extension
clean-sqlite:
	rm -f $(SQLITE_EXT) $(SQLITE_EXT).dSYM $(SQLITE_OBJS) $(SQLITE_DEPS)

# Prepare SQLite extension for distribution
dist-sqlite: $(DIST_DIR) $(SQLITE_EXT) $(SQLITE_HEADERS)
	mkdir -p $(DIST_DIR)/$(DIST_PACKAGE)/lib
	cp $(SQLITE_EXT) $(DIST_DIR)/$(DIST_PACKAGE)/lib/

.PHONY: clean-sqlite dist-sqlite 