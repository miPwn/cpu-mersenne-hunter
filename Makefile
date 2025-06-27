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

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y \
		build-essential \
		libgmp-dev \
		libfftw3-dev \
		libomp-dev \
		pkg-config

# Install dependencies (CentOS/RHEL/Fedora)
install-deps-rpm:
	@echo "Installing dependencies..."
	sudo dnf install -y \
		gcc-c++ \
		gmp-devel \
		fftw3-devel \
		libomp-devel \
		pkgconfig

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

# Memory usage analysis
profile-memory: $(TARGET)
	@echo "Running memory profiler..."
	valgrind --tool=massif --massif-out-file=massif.out $(TARGET) -p 607 -v
	ms_print massif.out > memory_profile.txt
	@echo "Memory profile saved to memory_profile.txt"

# CPU profiling
profile-cpu: 
	@$(MAKE) PROFILE=1 clean all
	@echo "Running CPU profiler..."
	@$(TARGET) -p 607 -v
	gprof $(TARGET) gmon.out > cpu_profile.txt
	@echo "CPU profile saved to cpu_profile.txt"

# Performance analysis with perf (Linux)
profile-perf: $(TARGET)
	@echo "Running perf analysis..."
	perf record -g $(TARGET) -p 607 -v
	perf report > perf_profile.txt
	@echo "Perf profile saved to perf_profile.txt"

# Optimized release build
release:
	@$(MAKE) clean
	@$(MAKE) CXX_FLAGS="$(CXX_FLAGS) -DNDEBUG -s"

# Debug build
debug:
	@$(MAKE) DEBUG=1

# Static analysis
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c++17 $(SRC_DIR)/
	clang-tidy $(SRC_DIR)/*.cpp -- $(INCLUDE_DIRS) $(CXX_FLAGS)

# Code formatting
format:
	@echo "Formatting code..."
	find $(SRC_DIR) -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Documentation generation
docs:
	@echo "Generating documentation..."
	doxygen Doxyfile

# Package for distribution
package: release
	@echo "Creating distribution package..."
	@mkdir -p mersenne_prime_package
	@cp $(TARGET) mersenne_prime_package/
	@cp README.md mersenne_prime_package/
	@cp Makefile mersenne_prime_package/
	@tar -czf mersenne_prime_$(shell date +%Y%m%d).tar.gz mersenne_prime_package/
	@rm -rf mersenne_prime_package/
	@echo "Package created: mersenne_prime_$(shell date +%Y%m%d).tar.gz"

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
	@echo "  profile-memory   - Analyze memory usage with valgrind"
	@echo "  profile-cpu      - Analyze CPU usage with gprof"
	@echo "  profile-perf     - Analyze performance with perf (Linux)"
	@echo "  analyze          - Run static code analysis"
	@echo "  format           - Format source code"
	@echo "  install-deps     - Install dependencies (Ubuntu/Debian)"
	@echo "  install-deps-rpm - Install dependencies (CentOS/RHEL/Fedora)"
	@echo "  package          - Create distribution package"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  DEBUG=1          - Enable debug build"
	@echo "  PROFILE=1        - Enable profiling build"
	@echo ""
	@echo "Examples:"
	@echo "  make             - Standard optimized build"
	@echo "  make debug       - Debug build with symbols"
	@echo "  make test        - Build and run tests"
	@echo "  make benchmark   - Build and run benchmarks"

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	@$(CXX) $(CXX_FLAGS) $(INCLUDE_DIRS) -MM -MT $(@:.d=.o) $< > $@

.PHONY: directories
