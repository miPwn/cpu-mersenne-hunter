# Mersenne Prime Calculator - High Performance Edition
# Makefile for optimized compilation

# Compiler settings
CXX = g++
CC = gcc

# Optimization flags for maximum performance
OPTIMIZE_FLAGS = -O3 -march=native -mtune=native -flto -ffast-math -funroll-loops
OPTIMIZE_FLAGS += -ftree-vectorize -fomit-frame-pointer -finline-functions

# Warning flags
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter

# C++ standard and language features
CXX_FLAGS = -std=c++17 $(WARNING_FLAGS) $(OPTIMIZE_FLAGS)

# Threading support
THREAD_FLAGS = -pthread -fopenmp

# Debug flags (use with make DEBUG=1)
ifeq ($(DEBUG), 1)
    DEBUG_FLAGS = -g -DDEBUG -O0 -fno-omit-frame-pointer
    CXX_FLAGS = -std=c++17 $(WARNING_FLAGS) $(DEBUG_FLAGS) $(THREAD_FLAGS)
else
    CXX_FLAGS += $(THREAD_FLAGS)
endif

# Profiling support (use with make PROFILE=1)
ifeq ($(PROFILE), 1)
    PROFILE_FLAGS = -pg -fno-omit-frame-pointer
    CXX_FLAGS += $(PROFILE_FLAGS)
endif

# Architecture-specific optimizations
ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
    CXX_FLAGS += -mavx2 -mfma -mbmi2
endif

# Include directories
INCLUDE_DIRS = -Isrc

# Library directories and linking
LIBS = -lgmp -lgmpxx -lfftw3 -lfftw3_threads -lm -ldl

# Source and object files
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Main executable
TARGET = $(BIN_DIR)/mersenne_prime

# Default target
.PHONY: all clean install test benchmark profile help

all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Main executable
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	@$(CXX) $(CXX_FLAGS) $(OBJECTS) -o $@ $(LIBS)
	@echo "Build complete: $@"

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@rm -f *.prof *.json *.csv gmon.out

# Install dependencies (Replit Nix environment)
install-deps:
	@echo "Dependencies should be installed via Replit's package manager..."
	@echo "Required packages: gcc, gmp, fftw, pkg-config"

# Quick tests
test: $(TARGET)
	@echo "Running quick tests..."
	@$(TARGET) -p 127 -v
	@$(TARGET) -p 521 -v

# Extended test suite
test-extended: $(TARGET)
	@echo "Running extended test suite..."
	@$(TARGET) -r 2 1000 -v -o test_results.txt

# Benchmark suite
benchmark: $(TARGET)
	@echo "Running performance benchmarks..."
	@$(TARGET) -b -s benchmark_profile.json

# Optimized release build
release:
	@$(MAKE) clean
	@$(MAKE) CXX_FLAGS="$(CXX_FLAGS) -DNDEBUG -s"

# Debug build
debug:
	@$(MAKE) DEBUG=1

# Help information
help:
	@echo "Mersenne Prime Calculator - Build System"
	@echo "========================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build the main executable (default)"
	@echo "  clean            - Remove build artifacts"
	@echo "  debug            - Build with debug information"
	@echo "  release          - Build optimized release version"
	@echo "  test             - Run quick functionality tests"
	@echo "  test-extended    - Run extended test suite"
	@echo "  benchmark        - Run performance benchmarks"
	@echo "  help             - Show this help message"

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	@$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -MM -MT $(@:.d=.o) $< > $@

.PHONY: directories