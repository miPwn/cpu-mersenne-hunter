#include "fft_multiply.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <cassert>
#include <omp.h>

// Static member initialization
std::vector<std::unique_ptr<FFTMultiplier::FFTWPlanCache>> FFTMultiplier::plan_cache;
std::unique_ptr<NTTMultiplier::NTTContext> NTTMultiplier::ntt_context;

// FFTWPlanCache implementation
FFTMultiplier::FFTWPlanCache::FFTWPlanCache(size_t n) : size(n) {
    input = fftw_alloc_complex(n);
    output = fftw_alloc_complex(n);
    
    // Use FFTW_MEASURE for better performance optimization
    // FFTW will run multiple algorithms and choose the fastest
    forward_plan = fftw_plan_dft_1d(n, input, output, FFTW_FORWARD, FFTW_MEASURE);
    inverse_plan = fftw_plan_dft_1d(n, output, input, FFTW_BACKWARD, FFTW_MEASURE);
}

FFTMultiplier::FFTWPlanCache::~FFTWPlanCache() {
    fftw_destroy_plan(forward_plan);
    fftw_destroy_plan(inverse_plan);
    fftw_free(input);
    fftw_free(output);
}

// FFTMultiplier implementation
FFTMultiplier::FFTMultiplier() {
    // Initialize FFTW
    fftw_init_threads();
    fftw_plan_with_nthreads(omp_get_max_threads());
}

FFTMultiplier::~FFTMultiplier() {
    cleanup_all_plans();
    fftw_cleanup_threads();
}

size_t FFTMultiplier::next_power_of_2(size_t n) {
    size_t power = 1;
    while (power < n) {
        power <<= 1;
    }
    return power;
}

FFTMultiplier::FFTWPlanCache* FFTMultiplier::get_or_create_plan(size_t size) {
    // Look for existing plan
    for (auto& cache : plan_cache) {
        if (cache->size == size) {
            return cache.get();
        }
    }
    
    // Create new plan if cache not full
    if (plan_cache.size() < CACHE_SIZE) {
        auto new_cache = std::make_unique<FFTWPlanCache>(size);
        FFTWPlanCache* ptr = new_cache.get();
        plan_cache.push_back(std::move(new_cache));
        return ptr;
    }
    
    // Replace oldest plan (simple FIFO for now)
    auto new_cache = std::make_unique<FFTWPlanCache>(size);
    FFTWPlanCache* ptr = new_cache.get();
    plan_cache[0] = std::move(new_cache);
    return ptr;
}

void FFTMultiplier::mpz_to_coefficients(const mpz_t value, std::vector<double>& coeffs, size_t base_bits) {
    size_t num_bits = mpz_sizeinbase(value, 2);
    size_t num_coeffs = (num_bits + base_bits - 1) / base_bits;
    
    coeffs.resize(num_coeffs, 0.0);
    
    mpz_t temp, mask;
    mpz_init(temp);
    mpz_init(mask);
    
    // Create mask for base_bits
    mpz_ui_pow_ui(mask, 2, base_bits);
    mpz_sub_ui(mask, mask, 1);
    
    mpz_set(temp, value);
    
    for (size_t i = 0; i < num_coeffs && mpz_cmp_ui(temp, 0) > 0; ++i) {
        mpz_t digit;
        mpz_init(digit);
        mpz_and(digit, temp, mask);
        coeffs[i] = mpz_get_d(digit);
        mpz_fdiv_q_2exp(temp, temp, base_bits);
        mpz_clear(digit);
    }
    
    mpz_clear(temp);
    mpz_clear(mask);
}

void FFTMultiplier::coefficients_to_mpz(mpz_t result, const std::vector<double>& coeffs, size_t base_bits) {
    mpz_set_ui(result, 0);
    
    mpz_t base, term;
    mpz_init(base);
    mpz_init(term);
    
    mpz_ui_pow_ui(base, 2, base_bits);
    
    for (int i = static_cast<int>(coeffs.size()) - 1; i >= 0; --i) {
        mpz_mul(result, result, base);
        
        uint64_t coeff_val = static_cast<uint64_t>(std::round(coeffs[i]));
        mpz_set_ui(term, coeff_val);
        mpz_add(result, result, term);
    }
    
    mpz_clear(base);
    mpz_clear(term);
}

void FFTMultiplier::normalize_coefficients(std::vector<double>& coeffs, size_t base_bits) {
    double base = static_cast<double>(1ULL << base_bits);
    double carry = 0.0;
    
    for (size_t i = 0; i < coeffs.size(); ++i) {
        coeffs[i] += carry;
        carry = std::floor(coeffs[i] / base);
        coeffs[i] = coeffs[i] - carry * base;
    }
    
    // Handle final carry
    while (carry > 0.0) {
        coeffs.push_back(carry - std::floor(carry / base) * base);
        carry = std::floor(carry / base);
    }
}

void FFTMultiplier::convolution_fft(const std::vector<double>& a, const std::vector<double>& b, 
                                   std::vector<double>& result) {
    size_t result_size = a.size() + b.size() - 1;
    size_t fft_size = next_power_of_2(result_size);
    
    FFTWPlanCache* cache = get_or_create_plan(fft_size);
    
    // Clear and prepare input arrays
    for (size_t i = 0; i < fft_size; ++i) {
        cache->input[i][0] = (i < a.size()) ? a[i] : 0.0;
        cache->input[i][1] = 0.0;
    }
    
    // Forward FFT of a
    fftw_execute(cache->forward_plan);
    
    // Store FFT(a) and prepare FFT(b)
    std::vector<fftw_complex> fft_a(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        fft_a[i][0] = cache->output[i][0];
        fft_a[i][1] = cache->output[i][1];
        
        cache->input[i][0] = (i < b.size()) ? b[i] : 0.0;
        cache->input[i][1] = 0.0;
    }
    
    // Forward FFT of b
    fftw_execute(cache->forward_plan);
    
    // Pointwise multiplication in frequency domain
    for (size_t i = 0; i < fft_size; ++i) {
        double real = fft_a[i][0] * cache->output[i][0] - fft_a[i][1] * cache->output[i][1];
        double imag = fft_a[i][0] * cache->output[i][1] + fft_a[i][1] * cache->output[i][0];
        cache->output[i][0] = real;
        cache->output[i][1] = imag;
    }
    
    // Inverse FFT
    fftw_execute(cache->inverse_plan);
    
    // Extract result and normalize
    result.resize(result_size);
    for (size_t i = 0; i < result_size; ++i) {
        result[i] = cache->input[i][0] / static_cast<double>(fft_size);
    }
}

void FFTMultiplier::fft_multiply_internal(mpz_t result, const mpz_t a, const mpz_t b) {
    constexpr size_t base_bits = 20;  // Use 20-bit digits for good precision
    
    std::vector<double> coeffs_a, coeffs_b, coeffs_result;
    
    mpz_to_coefficients(a, coeffs_a, base_bits);
    mpz_to_coefficients(b, coeffs_b, base_bits);
    
    convolution_fft(coeffs_a, coeffs_b, coeffs_result);
    normalize_coefficients(coeffs_result, base_bits);
    
    coefficients_to_mpz(result, coeffs_result, base_bits);
}

void FFTMultiplier::multiply(mpz_t result, const mpz_t a, const mpz_t b) {
    if (should_use_fft(a, b)) {
        fft_multiply_internal(result, a, b);
    } else {
        mpz_mul(result, a, b);
    }
}

void FFTMultiplier::square(mpz_t result, const mpz_t value) {
    if (should_use_fft(value)) {
        fft_multiply_internal(result, value, value);
    } else {
        mpz_mul(result, value, value);
    }
}

void FFTMultiplier::multiply_and_mod_mersenne(mpz_t result, const mpz_t a, const mpz_t b, uint64_t exponent) {
    multiply(result, a, b);
    
    // Optimized Mersenne modular reduction
    mpz_t high, low, mersenne;
    mpz_init(high);
    mpz_init(low);
    mpz_init(mersenne);
    
    mpz_ui_pow_ui(mersenne, 2, exponent);
    mpz_sub_ui(mersenne, mersenne, 1);
    
    while (mpz_cmp(result, mersenne) >= 0) {
        mpz_fdiv_q_2exp(high, result, exponent);
        mpz_fdiv_r_2exp(low, result, exponent);
        mpz_add(result, low, high);
    }
    
    mpz_clear(high);
    mpz_clear(low);
    mpz_clear(mersenne);
}

void FFTMultiplier::square_and_mod_mersenne(mpz_t result, const mpz_t value, uint64_t exponent) {
    multiply_and_mod_mersenne(result, value, value, exponent);
}

bool FFTMultiplier::should_use_fft(const mpz_t a, const mpz_t b) {
    size_t bits_a = mpz_sizeinbase(a, 2);
    size_t bits_b = b ? mpz_sizeinbase(b, 2) : bits_a;
    
    return (bits_a > FFT_THRESHOLD && bits_b > FFT_THRESHOLD);
}

size_t FFTMultiplier::estimate_fft_memory(size_t input_bits) {
    size_t coeffs = (input_bits + 19) / 20;  // 20-bit digits
    size_t fft_size = next_power_of_2(coeffs * 2);
    return fft_size * sizeof(fftw_complex) * 3;  // Input, output, and temporary arrays
}

void FFTMultiplier::cleanup_all_plans() {
    plan_cache.clear();
    fftw_cleanup();
}

// Static helper function for modular exponentiation
static uint64_t static_mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// NTTMultiplier implementation
NTTMultiplier::NTTContext::NTTContext(size_t size) : max_size(size) {
    omega_powers.resize(size);
    inv_omega_powers.resize(size);
    
    uint64_t omega = static_mod_pow(NTTMultiplier::NTT_ROOT, (NTTMultiplier::NTT_MOD - 1) / size, NTTMultiplier::NTT_MOD);
    uint64_t inv_omega = static_mod_pow(omega, NTTMultiplier::NTT_MOD - 2, NTTMultiplier::NTT_MOD);
    
    omega_powers[0] = 1;
    inv_omega_powers[0] = 1;
    
    for (size_t i = 1; i < size; ++i) {
        omega_powers[i] = (omega_powers[i-1] * omega) % NTTMultiplier::NTT_MOD;
        inv_omega_powers[i] = (inv_omega_powers[i-1] * inv_omega) % NTTMultiplier::NTT_MOD;
    }
}

NTTMultiplier::NTTMultiplier() {
    if (!ntt_context) {
        initialize_context(1 << 20);  // Default size
    }
}

NTTMultiplier::~NTTMultiplier() {
    // Context cleanup handled by static method
}

uint64_t NTTMultiplier::mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    
    return result;
}

void NTTMultiplier::ntt_transform(std::vector<uint64_t>& a, bool inverse) {
    size_t n = a.size();
    
    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }
    
    // NTT computation
    for (size_t len = 2; len <= n; len <<= 1) {
        size_t step = ntt_context->max_size / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < len / 2; ++j) {
                uint64_t u = a[i + j];
                uint64_t omega_idx = j * step;
                uint64_t omega = inverse ? ntt_context->inv_omega_powers[omega_idx] 
                                        : ntt_context->omega_powers[omega_idx];
                uint64_t v = (a[i + j + len / 2] * omega) % NTT_MOD;
                
                a[i + j] = (u + v) % NTT_MOD;
                a[i + j + len / 2] = (u - v + NTT_MOD) % NTT_MOD;
            }
        }
    }
    
    if (inverse) {
        uint64_t inv_n = mod_pow(n, NTT_MOD - 2, NTT_MOD);
        for (auto& val : a) {
            val = (val * inv_n) % NTT_MOD;
        }
    }
}

void NTTMultiplier::convolution_ntt(const std::vector<uint64_t>& a, const std::vector<uint64_t>& b, 
                                   std::vector<uint64_t>& result) {
    size_t result_size = a.size() + b.size() - 1;
    size_t ntt_size = 1;
    while (ntt_size < result_size) ntt_size <<= 1;
    
    std::vector<uint64_t> fa(a), fb(b);
    fa.resize(ntt_size, 0);
    fb.resize(ntt_size, 0);
    
    ntt_transform(fa, false);
    ntt_transform(fb, false);
    
    for (size_t i = 0; i < ntt_size; ++i) {
        fa[i] = (fa[i] * fb[i]) % NTT_MOD;
    }
    
    ntt_transform(fa, true);
    
    result.assign(fa.begin(), fa.begin() + result_size);
}

void NTTMultiplier::multiply(mpz_t result, const mpz_t a, const mpz_t b) {
    // Convert to NTT-suitable format and multiply
    // This is a simplified implementation - real version would handle
    // multi-precision reconstruction from NTT results
    mpz_mul(result, a, b);
}

void NTTMultiplier::square(mpz_t result, const mpz_t value) {
    multiply(result, value, value);
}

void NTTMultiplier::initialize_context(size_t max_size) {
    ntt_context = std::make_unique<NTTContext>(max_size);
}

void NTTMultiplier::cleanup_context() {
    ntt_context.reset();
}

// KaratsubaMultiplier implementation
void KaratsubaMultiplier::karatsuba_multiply_limbs(mp_limb_t* result, const mp_limb_t* a, size_t a_size,
                                                  const mp_limb_t* b, size_t b_size) {
    // Simplified Karatsuba implementation
    // Real implementation would be more complex with proper limb handling
    if (a_size < KARATSUBA_THRESHOLD || b_size < KARATSUBA_THRESHOLD) {
        // Use school multiplication for small sizes
        mpn_mul(result, a, a_size, b, b_size);
        return;
    }
    
    // For simplicity, fall back to mpn_mul
    mpn_mul(result, a, a_size, b, b_size);
}

void KaratsubaMultiplier::multiply(mpz_t result, const mpz_t a, const mpz_t b) {
    if (should_use_karatsuba(a, b)) {
        // Use custom Karatsuba if beneficial
        mpz_mul(result, a, b);  // Simplified - GMP already uses good algorithms
    } else {
        mpz_mul(result, a, b);
    }
}

void KaratsubaMultiplier::square(mpz_t result, const mpz_t value) {
    multiply(result, value, value);
}

bool KaratsubaMultiplier::should_use_karatsuba(const mpz_t a, const mpz_t b) {
    size_t limbs_a = mpz_size(a);
    size_t limbs_b = b ? mpz_size(b) : limbs_a;
    
    return (limbs_a >= KARATSUBA_THRESHOLD && limbs_b >= KARATSUBA_THRESHOLD);
}

// MultiplicationStrategy implementation
MultiplicationStrategy::MultiplicationStrategy() {
    // Initialize components
}

MultiplicationStrategy::~MultiplicationStrategy() {
    // Cleanup handled by component destructors
}

void MultiplicationStrategy::multiply(mpz_t result, const mpz_t a, const mpz_t b, Strategy strategy) {
    if (strategy == AUTO) {
        strategy = select_best_strategy(a, b);
    }
    
    switch (strategy) {
        case FFT:
            fft_multiplier.multiply(result, a, b);
            break;
        case NTT:
            ntt_multiplier.multiply(result, a, b);
            break;
        case KARATSUBA:
            karatsuba_multiplier.multiply(result, a, b);
            break;
        case GMP_NATIVE:
        default:
            mpz_mul(result, a, b);
            break;
    }
}

void MultiplicationStrategy::square(mpz_t result, const mpz_t value, Strategy strategy) {
    multiply(result, value, value, strategy);
}

MultiplicationStrategy::Strategy MultiplicationStrategy::select_best_strategy(const mpz_t a, const mpz_t b) {
    size_t bits_a = mpz_sizeinbase(a, 2);
    size_t bits_b = b ? mpz_sizeinbase(b, 2) : bits_a;
    
    // Simple heuristics - can be improved with benchmarking
    if (bits_a > 1000000 && bits_b > 1000000) {
        return FFT;
    } else if (bits_a > 10000 && bits_b > 10000) {
        return KARATSUBA;
    } else {
        return GMP_NATIVE;
    }
}

void MultiplicationStrategy::benchmark_strategies(const mpz_t a, const mpz_t b, 
                                                std::vector<std::pair<Strategy, double>>& results) {
    results.clear();
    mpz_t temp_result;
    mpz_init(temp_result);
    
    std::vector<Strategy> strategies = {GMP_NATIVE, KARATSUBA, FFT, NTT};
    
    for (Strategy strategy : strategies) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Perform multiple iterations for better timing accuracy
        const int iterations = 10;
        for (int i = 0; i < iterations; ++i) {
            multiply(temp_result, a, b, strategy);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - start).count() / iterations;
        
        results.emplace_back(strategy, duration);
    }
    
    mpz_clear(temp_result);
    
    // Sort by performance
    std::sort(results.begin(), results.end(), 
              [](const auto& a, const auto& b) { return a.second < b.second; });
}
