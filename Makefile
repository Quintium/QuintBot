# Check if the clang command is available
# Use the shell function to run the "which clang" command and store the output
CLANG = $(shell which clang)

# If the clang command is available, use it as the compiler
# Otherwise, use g++ as the default compiler
ifdef CLANG
	CXX = clang
else
	CXX = g++
endif

# Specify the target binary
BINARY = build/QuintBot

# Check the operating system
# Use different suffixes for the binary depending on the platform
ifeq ($(OS),Windows_NT)
	BINARY_SUFFIX = .exe
else
	BINARY_SUFFIX =
endif

# Add the suffix to the binary name
BINARY := $(BINARY)$(BINARY_SUFFIX)

# Specify the source files
SOURCES = $(wildcard src/*.cpp)

# Specify the compiler flags
CXXFLAGS = -std=c++20 -O3

# This is the default target of the makefile
# It specifies that the binary should be built from the source files
$(BINARY): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(BINARY) $(SOURCES)
