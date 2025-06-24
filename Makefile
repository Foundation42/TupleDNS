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
TEST_SOURCES = tests/test_tupledns.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_EXECUTABLE = tests/test_tupledns

# Examples
EXAMPLE_SOURCES = examples/example_basic.c examples/example_spatial.c examples/example_music.c
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

# Test executables
test: $(TEST_EXECUTABLE) test-comprehensive

$(TEST_EXECUTABLE): $(TEST_OBJECTS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -l$(LIB_NAME)
	@echo "Built test executable: $@"

# Comprehensive test suite
test-comprehensive: tests/c/test_comprehensive.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) -o tests/c/test_comprehensive $< -L. -l$(LIB_NAME)
	@echo "Built comprehensive test suite"

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
	emcc -O2 -s WASM=1 -s EXPORTED_FUNCTIONS='["_tupledns_init","_tupledns_register","_tupledns_find","_tupledns_free_result","_tupledns_cleanup","_tupledns_validate_coordinate","_tupledns_encode_coordinate","_tupledns_decode_coordinate"]' \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s MODULARIZE=1 \
		-s EXPORT_NAME="TupleDNSWasm" \
		$(SOURCES) -o tupledns-wasm.js

# Node.js server
server:
	node proxy-server.js

# Demo server with proxy
demo: server

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

# Test runners
test-all: test test-python test-javascript test-integration
	@echo "All tests completed"

test-python: $(STATIC_LIB)
	@echo "Running Python tests..."
	cd tests/python && python -m pytest test_tupledns.py -v

test-javascript: $(STATIC_LIB)
	@echo "Running JavaScript tests..."
	cd tests/javascript && node test_tupledns.js

test-integration: $(STATIC_LIB)
	@echo "Running integration tests..."
	python tests/integration/test_cross_language.py

test-memory: test-comprehensive
	@echo "Running memory leak detection..."
	valgrind --leak-check=full --error-exitcode=1 ./test_tupledns
	valgrind --leak-check=full --error-exitcode=1 ./tests/c/test_comprehensive

# Cleaning
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXAMPLE_OBJECTS)
	rm -f $(STATIC_LIB) $(SHARED_LIB) $(DYLIB)
	rm -f $(TEST_EXECUTABLE) $(EXAMPLE_EXECUTABLES)
	rm -f tests/c/test_comprehensive
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
	cp -r *.c *.h *.py *.js *.cjs *.html Makefile README.md LICENSE examples/ docs/ $(PACKAGE)/
	tar -czf $(PACKAGE).tar.gz $(PACKAGE)
	rm -rf $(PACKAGE)
	@echo "Created package: $(PACKAGE).tar.gz"

# Help
help:
	@echo "TupleDNS Build System"
	@echo ""
	@echo "Build Targets:"
	@echo "  all            - Build static and shared libraries"
	@echo "  shared         - Build shared library"
	@echo "  examples       - Build example programs"
	@echo "  python         - Build Python extension"
	@echo "  wasm           - Build WebAssembly version"
	@echo ""
	@echo "Test Targets:"
	@echo "  test           - Build and run basic tests"
	@echo "  test-all       - Run all test suites (C, Python, JS, Integration)"
	@echo "  test-python    - Run Python test suite"
	@echo "  test-javascript - Run JavaScript test suite"
	@echo "  test-integration - Run cross-language integration tests"
	@echo "  test-memory    - Run memory leak detection with valgrind"
	@echo ""
	@echo "Maintenance Targets:"
	@echo "  install        - Install library and headers"
	@echo "  uninstall      - Remove installed files"
	@echo "  clean          - Remove build artifacts"
	@echo "  format         - Format source code"
	@echo "  lint           - Run static analysis"
	@echo "  docs           - Generate documentation"
	@echo "  package        - Create distribution package"
	@echo "  help           - Show this help"

.PHONY: all shared test test-all test-python test-javascript test-integration test-memory test-comprehensive examples python wasm install uninstall clean format lint docs package help