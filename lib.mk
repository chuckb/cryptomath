# Header-only library files
LIB_HEADERS = $(INCLUDE_DIR)/cryptomath.h

# Library object files
LIB_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(LIB_HEADERS:.h=.o)))
LIB_DEPS = $(LIB_OBJS:.o=.d)

# Distribution files
DIST_LIB_HEADERS = $(addprefix $(DIST_DIR)/$(DIST_PACKAGE)/include/, $(notdir $(LIB_HEADERS)))

.PHONY: clean-lib dist-lib

# Clean library headers
clean-lib:
	rm -f $(LIB_OBJS) $(LIB_DEPS)

# Prepare library for distribution
dist-lib: $(DIST_DIR) $(LIB_HEADERS)
	mkdir -p $(DIST_DIR)/$(DIST_PACKAGE)/include
	cp $(LIB_HEADERS) $(DIST_DIR)/$(DIST_PACKAGE)/include/ 