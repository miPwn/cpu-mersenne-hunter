#ifndef FFT_MULTIPLY_HPP
#define FFT_MULTIPLY_HPP

#include <complex>
#include <vector>
#include <fftw3.h>
#include <gmp.h>
#include <memory>

class FFTMultiplier {
private:
    static constexpr double PI = 3.14159265358979323846;
    static constexpr size_t FFT_THRESHOLD = 100000;  // Bits threshold for using FFT
    static constexpr size_t CACHE_SIZE = 16;
    
    // FFTW plan cache for performance
    struct FFTWPlanCache {
        fftw_plan forward_plan;
        fftw_plan inverse_plan;
        size_t size;
        fftw_complex* input;
        fftw_complex* output;
        
        FFTWPlanCache(size_t n);
        ~FFTWPlanCache();
    };
    
    static std::vector<std::unique_ptr<FFTWPlanCache>> plan_cache;
    static size_t next_power_of_2(size_t n);
    static FFTWPlanCache* get_or_create_plan(size_t size);
    
    // Conversion utilities
    void mpz_to_coefficients(const mpz_t value, std::vector<double>& coeffs, size_t base_bits = 32);
    void coefficients_to_mpz(mpz_t result, const std::vector<double>& coeffs, size_t base_bits = 32);
    
    // Core FFT operations
    void convolution_fft(const std::vector<double>& a, const std::vector<double>& b, 
                        std::vector<double>& result);
    void fft_multiply_internal(mpz_t result, const mpz_t a, const mpz_t b);
    
    // Carry propagation for coefficient arrays
    void normalize_coefficients(std::vector<double>& coeffs, size_t base_bits = 32);
    
public:
    FFTMultiplier();
    ~FFTMultiplier();
    
    // Main multiplication interface
    void multiply(mpz_t result, const mpz_t a, const mpz_t b);
    void square(mpz_t result, const mpz_t value);
    
    // Mersenne-specific optimizations
    void multiply_and_mod_mersenne(mpz_t result, const mpz_t a, const mpz_t b, uint64_t exponent);
    void square_and_mod_mersenne(mpz_t result, const mpz_t value, uint64_t exponent);
    
    // Performance utilities
    static bool should_use_fft(const mpz_t a, const mpz_t b = nullptr);
    static size_t estimate_fft_memory(size_t input_bits);
    
    // Cleanup
    static void cleanup_all_plans();
};

// Number Theoretic Transform (NTT) alternative for exact integer arithmetic
class NTTMultiplier {
private:
    static constexpr uint64_t NTT_MOD = 998244353;  // Prime suitable for NTT
    static constexpr uint64_t NTT_ROOT = 3;         // Primitive root
    
    struct NTTContext {
        std::vector<uint64_t> omega_powers;
        std::vector<uint64_t> inv_omega_powers;
        size_t max_size;
        
        NTTContext(size_t size);
    };
    
    static std::unique_ptr<NTTContext> ntt_context;
    
    uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod);
    void ntt_transform(std::vector<uint64_t>& a, bool inverse = false);
    void convolution_ntt(const std::vector<uint64_t>& a, const std::vector<uint64_t>& b, 
                        std::vector<uint64_t>& result);
    
public:
    NTTMultiplier();
    ~NTTMultiplier();
    
    void multiply(mpz_t result, const mpz_t a, const mpz_t b);
    void square(mpz_t result, const mpz_t value);
    
    static void initialize_context(size_t max_size);
    static void cleanup_context();
};

// Karatsuba multiplication for medium-sized numbers
class KaratsubaMultiplier {
private:
    static constexpr size_t KARATSUBA_THRESHOLD = 1000;  // Limbs threshold
    
    void karatsuba_multiply_limbs(mp_limb_t* result, const mp_limb_t* a, size_t a_size,
                                 const mp_limb_t* b, size_t b_size);
    
public:
    void multiply(mpz_t result, const mpz_t a, const mpz_t b);
    void square(mpz_t result, const mpz_t value);
    
    static bool should_use_karatsuba(const mpz_t a, const mpz_t b = nullptr);
};

// Multiplication strategy selector
class MultiplicationStrategy {
private:
    FFTMultiplier fft_multiplier;
    NTTMultiplier ntt_multiplier;
    KaratsubaMultiplier karatsuba_multiplier;
    
public:
    enum Strategy {
        AUTO,
        GMP_NATIVE,
        KARATSUBA,
        FFT,
        NTT
    };
    
    MultiplicationStrategy();
    ~MultiplicationStrategy();
    
    void multiply(mpz_t result, const mpz_t a, const mpz_t b, Strategy strategy = AUTO);
    void square(mpz_t result, const mpz_t value, Strategy strategy = AUTO);
    
    Strategy select_best_strategy(const mpz_t a, const mpz_t b = nullptr);
    
    // Performance benchmarking
    void benchmark_strategies(const mpz_t a, const mpz_t b, 
                            std::vector<std::pair<Strategy, double>>& results);
};

#endif // FFT_MULTIPLY_HPP
