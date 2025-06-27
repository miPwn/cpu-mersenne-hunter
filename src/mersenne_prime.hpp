#ifndef MERSENNE_PRIME_HPP
#define MERSENNE_PRIME_HPP

#include <gmp.h>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include "big_integer.hpp"
#include "performance_profiler.hpp"

class MersennePrime {
private:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    static constexpr size_t MIN_PARALLEL_EXPONENT = 1000000;
    
    // Thread-local storage for GMP variables
    struct ThreadContext {
        mpz_t s, temp, mersenne, temp2;
        ThreadContext();
        ~ThreadContext();
    };
    
    static thread_local std::unique_ptr<ThreadContext> tls_context;
    
    // Performance counters
    std::atomic<uint64_t> iterations_completed{0};
    std::atomic<uint64_t> fft_operations{0};
    
    PerformanceProfiler& profiler;
    
    // Core algorithm implementations
    bool lucas_lehmer_sequential(uint64_t exponent);
    bool lucas_lehmer_parallel(uint64_t exponent, int num_threads);
    
    // Optimized modular operations
    void mod_mersenne_optimized(mpz_t result, const mpz_t value, uint64_t exponent);
    void square_mod_mersenne(mpz_t result, const mpz_t value, uint64_t exponent);
    
    // Memory management optimizations
    void precompute_constants(uint64_t exponent);
    void cleanup_thread_context();
    
public:
    explicit MersennePrime(PerformanceProfiler& prof);
    ~MersennePrime();
    
    // Main interface
    bool is_mersenne_prime(uint64_t exponent, int num_threads = 0);
    
    // Batch processing for multiple exponents
    std::vector<uint64_t> find_mersenne_primes_range(uint64_t start, uint64_t end, int num_threads = 0);
    
    // Performance metrics
    uint64_t get_iterations_completed() const { return iterations_completed.load(); }
    uint64_t get_fft_operations() const { return fft_operations.load(); }
    
    // Static utility functions
    static bool is_prime(uint64_t n);
    static uint64_t estimate_memory_usage(uint64_t exponent);
    static uint64_t estimate_computation_time(uint64_t exponent);
};

// Specialized optimizations for specific exponent ranges
namespace MersenneOptimizations {
    // For exponents < 1M
    bool fast_lucas_lehmer_small(uint64_t exponent);
    
    // For exponents 1M - 10M
    bool fast_lucas_lehmer_medium(uint64_t exponent);
    
    // For exponents > 10M
    bool fast_lucas_lehmer_large(uint64_t exponent, int num_threads);
}

#endif // MERSENNE_PRIME_HPP
