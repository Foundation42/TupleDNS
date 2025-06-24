# TupleDNS Makefile
# Build system for the TupleDNS library

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -fPIC
LDFLAGS = -shared
INCLUDES = -I.

# Library name
LIB_NAME = tupledns
STATIC_LIB = lib$(LIB_NAME).a
SHARED_LIB = lib$(LIB_NAME).so
DYLIB = lib$(LIB_NAME).dylib

# Source files
SOURCES = tupledns.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = tupledns.h

# Test files
TEST_SOURCES = test_tupledns.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_EXECUTABLE = test_tupledns

# Examples
EXAMPLE_SOURCES = example_basic.c example_spatial.c example_music.c
EXAMPLE_OBJECTS = $(EXAMPLE_SOURCES:.c=.o)
EXAMPLE_EXECUTABLES = $(EXAMPLE_SOURCES:.c=)

# Default target
all: $(STATIC_LIB) shared

# Static library
$(STATIC_LIB): $(OBJECTS)
	ar rcs $@ $^
	@echo "Built static library: $@"

# Shared library (Linux)
$(SHARED_LIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Built shared library: $@"

# Dynamic library (macOS)
$(DYLIB): $(OBJECTS)
	$(CC) -dynamiclib -o $@ $^
	@echo "Built dynamic library: $@"

# Determine shared library target based on OS
shared:
ifeq ($(shell uname -s),Darwin)
	$(MAKE) $(DYLIB)
else
	$(MAKE) $(SHARED_LIB)
endif

# Object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test executable
test: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_OBJECTS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -l$(LIB_NAME)
	@echo "Built test executable: $@"

# Examples
examples: $(EXAMPLE_EXECUTABLES)

$(EXAMPLE_EXECUTABLES): %: %.o $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -l$(LIB_NAME)
	@echo "Built example: $@"

# Python extension
python: shared
	python3 setup.py build_ext --inplace

# WebAssembly build
wasm:
	emcc -O2 -s WASM=1 -s EXPORTED_FUNCTIONS='["_tupledns_init","_tupledns_register","_tupledns_find","_tupledns_free_result","_tupledns_cleanup"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		$(SOURCES) -o tupledns.js

# Install targets
PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

install: all
	install -d $(LIBDIR) $(INCLUDEDIR)
	install -m 644 $(STATIC_LIB) $(LIBDIR)/
	install -m 644 $(HEADERS) $(INCLUDEDIR)/
ifeq ($(shell uname -s),Darwin)
	install -m 755 $(DYLIB) $(LIBDIR)/
else
	install -m 755 $(SHARED_LIB) $(LIBDIR)/
endif
	@echo "Installed TupleDNS to $(PREFIX)"

uninstall:
	rm -f $(LIBDIR)/$(STATIC_LIB)
	rm -f $(LIBDIR)/$(SHARED_LIB)
	rm -f $(LIBDIR)/$(DYLIB)
	rm -f $(INCLUDEDIR)/tupledns.h
	@echo "Uninstalled TupleDNS from $(PREFIX)"

# Cleaning
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXAMPLE_OBJECTS)
	rm -f $(STATIC_LIB) $(SHARED_LIB) $(DYLIB)
	rm -f $(TEST_EXECUTABLE) $(EXAMPLE_EXECUTABLES)
	rm -f tupledns.js tupledns.wasm
	rm -rf build/
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete
	@echo "Cleaned build artifacts"

# Development helpers
format:
	clang-format -i $(SOURCES) $(HEADERS) $(TEST_SOURCES) $(EXAMPLE_SOURCES)

lint:
	cppcheck --enable=all --std=c99 $(SOURCES) $(HEADERS)

# Documentation
docs:
	doxygen Doxyfile

# Package
VERSION ?= 1.0.0
PACKAGE = tupledns-$(VERSION)

package: clean
	mkdir -p $(PACKAGE)
	cp -r *.c *.h *.py Makefile README.md LICENSE examples/ $(PACKAGE)/
	tar -czf $(PACKAGE).tar.gz $(PACKAGE)
	rm -rf $(PACKAGE)
	@echo "Created package: $(PACKAGE).tar.gz"

# Help
help:
	@echo "TupleDNS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build static and shared libraries"
	@echo "  shared     - Build shared library"
	@echo "  test       - Build test executable"
	@echo "  examples   - Build example programs"
	@echo "  python     - Build Python extension"
	@echo "  wasm       - Build WebAssembly version"
	@echo "  install    - Install library and headers"
	@echo "  uninstall  - Remove installed files"
	@echo "  clean      - Remove build artifacts"
	@echo "  format     - Format source code"
	@echo "  lint       - Run static analysis"
	@echo "  docs       - Generate documentation"
	@echo "  package    - Create distribution package"
	@echo "  help       - Show this help"

.PHONY: all shared test examples python wasm install uninstall clean format lint docs package help