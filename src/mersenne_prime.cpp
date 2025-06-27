#include "mersenne_prime.hpp"
#include "fft_multiply.hpp"
#include <omp.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

thread_local std::unique_ptr<MersennePrime::ThreadContext> MersennePrime::tls_context;

MersennePrime::ThreadContext::ThreadContext() {
    mpz_init(s);
    mpz_init(temp);
    mpz_init(mersenne);
    mpz_init(temp2);
}

MersennePrime::ThreadContext::~ThreadContext() {
    mpz_clear(s);
    mpz_clear(temp);
    mpz_clear(mersenne);
    mpz_clear(temp2);
}

MersennePrime::MersennePrime(PerformanceProfiler& prof) : profiler(prof) {
    // Initialize GMP random state for thread safety
    gmp_randinit_default(BigInteger::get_random_state());
}

MersennePrime::~MersennePrime() {
    cleanup_thread_context();
}

void MersennePrime::cleanup_thread_context() {
    if (tls_context) {
        tls_context.reset();
    }
}

bool MersennePrime::is_mersenne_prime(uint64_t exponent, int num_threads) {
    profiler.start_timer("total_computation");
    
    // Input validation
    if (exponent < 2 || !is_prime(exponent)) {
        profiler.end_timer("total_computation");
        return false;
    }
    
    bool result;
    
    // Choose algorithm based on exponent size and threading preference
    if (num_threads <= 1 || exponent < MIN_PARALLEL_EXPONENT) {
        result = lucas_lehmer_sequential(exponent);
    } else {
        if (num_threads == 0) {
            num_threads = std::thread::hardware_concurrency();
        }
        result = lucas_lehmer_parallel(exponent, num_threads);
    }
    
    profiler.end_timer("total_computation");
    return result;
}

bool MersennePrime::lucas_lehmer_sequential(uint64_t exponent) {
    profiler.start_timer("lucas_lehmer_sequential");
    
    // Initialize thread context if not already done
    if (!tls_context) {
        tls_context = std::make_unique<ThreadContext>();
    }
    
    auto& ctx = *tls_context;
    
    // Compute M_p = 2^p - 1
    mpz_ui_pow_ui(ctx.mersenne, 2, exponent);
    mpz_sub_ui(ctx.mersenne, ctx.mersenne, 1);
    
    // Initialize s = 4
    mpz_set_ui(ctx.s, 4);
    
    // Perform Lucas-Lehmer iterations: s = (s^2 - 2) mod M_p
    for (uint64_t i = 0; i < exponent - 2; ++i) {
        profiler.start_timer("ll_iteration");
        
        // s = s^2
        square_mod_mersenne(ctx.temp, ctx.s, exponent);
        
        // s = s - 2
        mpz_sub_ui(ctx.temp, ctx.temp, 2);
        
        // s = s mod M_p (optimized for Mersenne numbers)
        mod_mersenne_optimized(ctx.s, ctx.temp, exponent);
        
        iterations_completed.fetch_add(1);
        profiler.end_timer("ll_iteration");
    }
    
    // Check if s == 0
    bool is_prime = mpz_cmp_ui(ctx.s, 0) == 0;
    
    profiler.end_timer("lucas_lehmer_sequential");
    return is_prime;
}

bool MersennePrime::lucas_lehmer_parallel(uint64_t exponent, int num_threads) {
    profiler.start_timer("lucas_lehmer_parallel");
    
    // For very large exponents, we can parallelize the modular arithmetic
    // This is a simplified parallel approach - in practice, more sophisticated
    // techniques like parallel modular exponentiation would be used
    
    omp_set_num_threads(num_threads);
    
    mpz_t mersenne, s, temp;
    mpz_init(mersenne);
    mpz_init(s);
    mpz_init(temp);
    
    // Compute M_p = 2^p - 1
    mpz_ui_pow_ui(mersenne, 2, exponent);
    mpz_sub_ui(mersenne, mersenne, 1);
    
    // Initialize s = 4
    mpz_set_ui(s, 4);
    
    // Parallel Lucas-Lehmer iterations
    #pragma omp parallel
    {
        mpz_t local_temp, local_s;
        mpz_init(local_temp);
        mpz_init(local_s);
        
        #pragma omp for schedule(static)
        for (uint64_t i = 0; i < exponent - 2; ++i) {
            #pragma omp critical
            {
                mpz_set(local_s, s);
            }
            
            // s = s^2
            square_mod_mersenne(local_temp, local_s, exponent);
            
            // s = s - 2
            mpz_sub_ui(local_temp, local_temp, 2);
            
            // s = s mod M_p
            mod_mersenne_optimized(local_s, local_temp, exponent);
            
            #pragma omp critical
            {
                mpz_set(s, local_s);
            }
            
            iterations_completed.fetch_add(1);
        }
        
        mpz_clear(local_temp);
        mpz_clear(local_s);
    }
    
    // Check if s == 0
    bool is_prime = mpz_cmp_ui(s, 0) == 0;
    
    mpz_clear(mersenne);
    mpz_clear(s);
    mpz_clear(temp);
    
    profiler.end_timer("lucas_lehmer_parallel");
    return is_prime;
}

void MersennePrime::square_mod_mersenne(mpz_t result, const mpz_t value, uint64_t exponent) {
    profiler.start_timer("square_mod_mersenne");
    
    // Use FFT multiplication for large numbers
    if (mpz_sizeinbase(value, 2) > 1000000) {
        FFTMultiplier multiplier;
        multiplier.multiply_and_mod_mersenne(result, value, value, exponent);
        fft_operations.fetch_add(1);
    } else {
        // Use GMP's optimized multiplication for smaller numbers
        mpz_mul(result, value, value);
        mod_mersenne_optimized(result, result, exponent);
    }
    
    profiler.end_timer("square_mod_mersenne");
}

void MersennePrime::mod_mersenne_optimized(mpz_t result, const mpz_t value, uint64_t exponent) {
    profiler.start_timer("mod_mersenne");
    
    // Optimized modular reduction for Mersenne numbers
    // M_p = 2^p - 1, so x mod M_p = (x mod 2^p) + (x >> p)
    // Repeat until result < M_p
    
    mpz_t high, low, mersenne;
    mpz_init(high);
    mpz_init(low);
    mpz_init(mersenne);
    
    // Compute M_p = 2^p - 1
    mpz_ui_pow_ui(mersenne, 2, exponent);
    mpz_sub_ui(mersenne, mersenne, 1);
    
    mpz_set(result, value);
    
    while (mpz_cmp(result, mersenne) >= 0) {
        // Split result into high and low parts
        mpz_fdiv_q_2exp(high, result, exponent);  // high = result >> p
        mpz_fdiv_r_2exp(low, result, exponent);   // low = result & ((1 << p) - 1)
        
        // result = low + high
        mpz_add(result, low, high);
    }
    
    mpz_clear(high);
    mpz_clear(low);
    mpz_clear(mersenne);
    
    profiler.end_timer("mod_mersenne");
}

std::vector<uint64_t> MersennePrime::find_mersenne_primes_range(uint64_t start, uint64_t end, int num_threads) {
    profiler.start_timer("range_search");
    
    std::vector<uint64_t> primes;
    std::vector<uint64_t> candidates;
    
    // Collect prime candidates
    for (uint64_t p = start; p <= end; p += 2) {
        if (is_prime(p)) {
            candidates.push_back(p);
        }
    }
    
    // Test candidates in parallel
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < candidates.size(); ++i) {
        uint64_t p = candidates[i];
        if (is_mersenne_prime(p, 1)) {  // Use single thread per candidate
            #pragma omp critical
            {
                primes.push_back(p);
                std::cout << "Found Mersenne prime: M" << p << std::endl;
            }
        }
    }
    
    std::sort(primes.begin(), primes.end());
    
    profiler.end_timer("range_search");
    return primes;
}

bool MersennePrime::is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    uint64_t sqrt_n = static_cast<uint64_t>(std::sqrt(n));
    for (uint64_t i = 3; i <= sqrt_n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

uint64_t MersennePrime::estimate_memory_usage(uint64_t exponent) {
    // Estimate memory usage in bytes for Mersenne number M_p = 2^p - 1
    uint64_t bits = exponent;
    uint64_t bytes_per_number = (bits + 7) / 8;  // Round up to nearest byte
    
    // Account for multiple temporary variables in computation
    uint64_t total_memory = bytes_per_number * 8;  // 8 big integers typically needed
    
    return total_memory;
}

uint64_t MersennePrime::estimate_computation_time(uint64_t exponent) {
    // Rough estimate based on O(p^2 * log(p)) complexity
    // Returns estimated time in milliseconds
    double operations = static_cast<double>(exponent - 2);  // Lucas-Lehmer iterations
    double bits_per_iteration = static_cast<double>(exponent);
    double complexity_factor = operations * bits_per_iteration * std::log2(bits_per_iteration);
    
    // Assume 1 billion operations per second (very rough estimate)
    return static_cast<uint64_t>(complexity_factor / 1e9 * 1000);
}

// Optimized implementations for different exponent ranges
namespace MersenneOptimizations {
    
bool fast_lucas_lehmer_small(uint64_t exponent) {
    // For small exponents, use native integer arithmetic when possible
    if (exponent > 63) return false;  // Fallback to GMP for larger exponents
    
    uint64_t mersenne = (1ULL << exponent) - 1;
    uint64_t s = 4;
    
    for (uint64_t i = 0; i < exponent - 2; ++i) {
        s = ((s * s) - 2) % mersenne;
    }
    
    return s == 0;
}

bool fast_lucas_lehmer_medium(uint64_t exponent) {
    // Use GMP with optimized settings for medium-sized exponents
    mpz_t s, mersenne, temp;
    mpz_init(s);
    mpz_init(mersenne);
    mpz_init(temp);
    
    // Compute M_p = 2^p - 1
    mpz_ui_pow_ui(mersenne, 2, exponent);
    mpz_sub_ui(mersenne, mersenne, 1);
    
    // Initialize s = 4
    mpz_set_ui(s, 4);
    
    // Lucas-Lehmer iterations with medium optimization
    for (uint64_t i = 0; i < exponent - 2; ++i) {
        mpz_mul(temp, s, s);
        mpz_sub_ui(temp, temp, 2);
        mpz_mod(s, temp, mersenne);
    }
    
    bool result = mpz_cmp_ui(s, 0) == 0;
    
    mpz_clear(s);
    mpz_clear(mersenne);
    mpz_clear(temp);
    
    return result;
}

bool fast_lucas_lehmer_large(uint64_t exponent, int num_threads) {
    // For large exponents, use FFT-based multiplication and advanced parallelization
    // This would integrate with the main MersennePrime class
    MersennePrime mp(PerformanceProfiler::get_instance());
    return mp.is_mersenne_prime(exponent, num_threads);
}

} // namespace MersenneOptimizations
