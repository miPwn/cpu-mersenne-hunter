# Mersenne Prime Calculator - High Performance Edition

## Overview

This is a high-performance C++ application designed to calculate Mersenne primes using advanced mathematical algorithms and optimization techniques. The application implements the Lucas-Lehmer test with FFT-based multiplication, multi-threading support, and comprehensive performance profiling. It's built for computational efficiency on modern multi-core systems and can handle very large exponents (up to 100 million+).

## System Architecture

### Core Architecture Pattern
- **Modular Design**: Clean separation of concerns with distinct components for big integer operations, FFT multiplication, Mersenne prime testing, and performance profiling
- **High-Performance Computing Focus**: Designed for maximum computational efficiency with cache-friendly memory access patterns and vectorization support
- **Thread-Safe Implementation**: Multi-threaded execution with OpenMP parallelization and thread-local storage for GMP variables

### Technology Stack
- **Language**: C++17 with GCC compiler optimizations
- **Mathematical Libraries**: 
  - GMP (GNU Multiple Precision Arithmetic Library) for big integer operations
  - FFTW3 (Fastest Fourier Transform in the West) for FFT-based multiplication
- **Parallelization**: OpenMP for multi-threading support
- **Build System**: GNU Make with advanced optimization flags

## Key Components

### 1. Big Integer Operations (`big_integer.hpp/cpp`)
- **Purpose**: Wrapper around GMP for high-precision arithmetic operations
- **Features**: Memory pool management, operator overloading, move semantics
- **Optimization**: Static memory pools to reduce allocation overhead

### 2. FFT Multiplication (`fft_multiply.hpp/cpp`)
- **Purpose**: Fast multiplication of very large numbers using FFT algorithms
- **Features**: FFTW plan caching, automatic algorithm selection, NTT (Number Theoretic Transform) support
- **Optimization**: Cached FFT plans for repeated operations, threshold-based algorithm switching

### 3. Mersenne Prime Calculator (`mersenne_prime.hpp/cpp`)
- **Purpose**: Core Lucas-Lehmer test implementation for Mersenne prime detection
- **Features**: Sequential and parallel implementations, optimized modular arithmetic
- **Optimization**: Thread-local storage, cache-friendly memory access, specialized Mersenne number operations

### 4. Performance Profiler (`performance_profiler.hpp/cpp`)
- **Purpose**: Comprehensive performance monitoring and benchmarking
- **Features**: Timing analysis, memory usage tracking, system capability detection
- **Output**: JSON-formatted performance reports

### 5. Main Application (`main.cpp`)
- **Purpose**: Command-line interface and program orchestration
- **Features**: Argument parsing, result output, benchmark mode
- **Usage**: Supports single prime testing, range testing, and benchmarking

## Data Flow

### Primary Execution Flow
1. **Input Processing**: Command-line arguments parsed for exponent(s) and options
2. **System Detection**: Hardware capabilities detected for optimization
3. **Algorithm Selection**: Sequential vs parallel Lucas-Lehmer based on exponent size
4. **Core Computation**: 
   - Lucas-Lehmer iterations with FFT-based squaring operations
   - Modular reduction using optimized Mersenne number properties
5. **Result Output**: Prime/composite determination with optional performance metrics

### Memory Management
- **GMP Integration**: Automatic memory management through RAII wrappers
- **Memory Pools**: Static pools for frequently allocated GMP objects
- **Thread Safety**: Thread-local storage for per-thread GMP variables

## External Dependencies

### Required Libraries
- **GMP**: For arbitrary precision arithmetic operations
- **FFTW3**: For fast Fourier transform computations
- **OpenMP**: For parallel processing support

### System Requirements
- **OS**: Linux (Ubuntu 18.04+, CentOS 7+)
- **Compiler**: GCC 7+ with C++17 support
- **CPU**: Modern x86_64 processor with AVX2 support recommended
- **Memory**: Minimum 8GB RAM, 32GB+ recommended for large exponents

## Deployment Strategy

### Build Configuration
- **Optimization Level**: Maximum performance (-O3, -march=native, -flto)
- **Vectorization**: Auto-vectorization with AVX2 instructions
- **Debug Support**: Conditional debug builds with make DEBUG=1
- **Profiling**: Optional profiling support with make PROFILE=1

### Installation Process
1. **Dependencies**: Automated installation of required libraries
2. **Compilation**: Optimized build with architecture-specific flags
3. **Execution**: Direct binary execution with command-line options

### Performance Tuning
- **Thread Count**: Automatic detection of optimal thread count
- **Algorithm Thresholds**: Automatic selection between multiplication strategies
- **Memory Management**: Pool-based allocation for reduced overhead

## Changelog

```
Changelog:
- June 27, 2025. Initial setup
```

## User Preferences

```
Preferred communication style: Simple, everyday language.
```