# Mersenne Prime Calculator - High Performance Edition

A highly optimized C++ application for calculating Mersenne primes using FFT-based Lucas-Lehmer tests, multi-threading, and advanced performance optimization techniques.

## Features

### Core Capabilities
- **Lucas-Lehmer Test Implementation**: Optimized for very large exponents (up to 100 million+)
- **FFT-Based Multiplication**: Uses FFTW for fast multiplication of large numbers
- **Multi-Threading Support**: Utilizes all CPU cores with OpenMP parallelization
- **Advanced Big Integer Operations**: Memory-efficient GMP-based arithmetic
- **Performance Profiling**: Comprehensive benchmarking and analysis tools

### Optimization Techniques
- **Cache-Friendly Memory Access**: Optimized data structures and access patterns
- **Vectorization**: Auto-vectorization with compiler intrinsics
- **Modular Arithmetic Optimizations**: Specialized Mersenne number operations
- **Algorithmic Selection**: Automatic choice between multiplication strategies
- **Memory Pool Management**: Reduced allocation overhead

## Requirements

### System Requirements
- **OS**: Linux (Ubuntu 18.04+, CentOS 7+, or equivalent)
- **CPU**: Modern multi-core x86_64 processor (Intel Xeon, AMD EPYC recommended)
- **RAM**: Minimum 8GB, 32GB+ recommended for large exponents
- **Compiler**: GCC 7+ or Clang 6+ with C++17 support

### Dependencies
- **GMP**: GNU Multiple Precision Arithmetic Library
- **FFTW3**: Fastest Fourier Transform in the West
- **OpenMP**: Parallel programming support

## Installation

### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential libgmp-dev libfftw3-dev libomp-dev pkg-config

# Build the application
make install-deps  # Alternative dependency installation
make all
