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

# These are the targets of the makefile
# They specify that the binary should be built from the source files using the given compiler
clang: $(SOURCES)
	clang $(CXXFLAGS) -o $(BINARY) $(SOURCES)

g++: $(SOURCES)
	g++ $(CXXFLAGS) -o $(BINARY) $(SOURCES)
