# Header-only library files
LIB_HEADERS = $(INCLUDE_DIR)/cryptomath.h

.PHONY: clean-lib

# Clean library headers
clean-lib:
	@echo "Header-only library - no cleanup needed" 