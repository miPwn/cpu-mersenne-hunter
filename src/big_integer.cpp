#include "big_integer.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

// Static member initialization
gmp_randstate_t BigInteger::random_state;
bool BigInteger::random_state_initialized = false;
std::vector<mpz_t*> BigInteger::memory_pool;
size_t BigInteger::pool_index = 0;

BigInteger::BigInteger() {
    mpz_init(value);
}

BigInteger::BigInteger(long long val) {
    mpz_init(value);
    mpz_set_si(value, val);
}

BigInteger::BigInteger(const std::string& str, int base) {
    mpz_init(value);
    if (mpz_set_str(value, str.c_str(), base) != 0) {
        mpz_clear(value);
        throw std::invalid_argument("Invalid string representation for BigInteger");
    }
}

BigInteger::BigInteger(const BigInteger& other) {
    mpz_init(value);
    mpz_set(value, other.value);
}

BigInteger::BigInteger(BigInteger&& other) noexcept {
    mpz_init(value);
    mpz_swap(value, other.value);
}

BigInteger::~BigInteger() {
    mpz_clear(value);
}

BigInteger& BigInteger::operator=(const BigInteger& other) {
    if (this != &other) {
        mpz_set(value, other.value);
    }
    return *this;
}

BigInteger& BigInteger::operator=(BigInteger&& other) noexcept {
    if (this != &other) {
        mpz_swap(value, other.value);
    }
    return *this;
}

BigInteger& BigInteger::operator=(long long val) {
    mpz_set_si(value, val);
    return *this;
}

// Arithmetic operators
BigInteger BigInteger::operator+(const BigInteger& other) const {
    BigInteger result;
    mpz_add(result.value, value, other.value);
    return result;
}

BigInteger BigInteger::operator-(const BigInteger& other) const {
    BigInteger result;
    mpz_sub(result.value, value, other.value);
    return result;
}

BigInteger BigInteger::operator*(const BigInteger& other) const {
    BigInteger result;
    mpz_mul(result.value, value, other.value);
    return result;
}

BigInteger BigInteger::operator/(const BigInteger& other) const {
    if (mpz_cmp_ui(other.value, 0) == 0) {
        throw std::domain_error("Division by zero");
    }
    BigInteger result;
    mpz_fdiv_q(result.value, value, other.value);
    return result;
}

BigInteger BigInteger::operator%(const BigInteger& other) const {
    if (mpz_cmp_ui(other.value, 0) == 0) {
        throw std::domain_error("Modulo by zero");
    }
    BigInteger result;
    mpz_fdiv_r(result.value, value, other.value);
    return result;
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
    mpz_add(value, value, other.value);
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other) {
    mpz_sub(value, value, other.value);
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
    mpz_mul(value, value, other.value);
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
    if (mpz_cmp_ui(other.value, 0) == 0) {
        throw std::domain_error("Division by zero");
    }
    mpz_fdiv_q(value, value, other.value);
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
    if (mpz_cmp_ui(other.value, 0) == 0) {
        throw std::domain_error("Modulo by zero");
    }
    mpz_fdiv_r(value, value, other.value);
    return *this;
}

// Comparison operators
bool BigInteger::operator==(const BigInteger& other) const {
    return mpz_cmp(value, other.value) == 0;
}

bool BigInteger::operator!=(const BigInteger& other) const {
    return mpz_cmp(value, other.value) != 0;
}

bool BigInteger::operator<(const BigInteger& other) const {
    return mpz_cmp(value, other.value) < 0;
}

bool BigInteger::operator<=(const BigInteger& other) const {
    return mpz_cmp(value, other.value) <= 0;
}

bool BigInteger::operator>(const BigInteger& other) const {
    return mpz_cmp(value, other.value) > 0;
}

bool BigInteger::operator>=(const BigInteger& other) const {
    return mpz_cmp(value, other.value) >= 0;
}

// Bitwise operations
BigInteger BigInteger::operator<<(unsigned long shift) const {
    BigInteger result;
    mpz_mul_2exp(result.value, value, shift);
    return result;
}

BigInteger BigInteger::operator>>(unsigned long shift) const {
    BigInteger result;
    mpz_fdiv_q_2exp(result.value, value, shift);
    return result;
}

BigInteger& BigInteger::operator<<=(unsigned long shift) {
    mpz_mul_2exp(value, value, shift);
    return *this;
}

BigInteger& BigInteger::operator>>=(unsigned long shift) {
    mpz_fdiv_q_2exp(value, value, shift);
    return *this;
}

// Mathematical functions
BigInteger BigInteger::pow(unsigned long exponent) const {
    BigInteger result;
    mpz_pow_ui(result.value, value, exponent);
    return result;
}

BigInteger BigInteger::mod_pow(const BigInteger& exponent, const BigInteger& modulus) const {
    BigInteger result;
    mpz_powm(result.value, value, exponent.value, modulus.value);
    return result;
}

BigInteger BigInteger::sqrt() const {
    BigInteger result;
    mpz_sqrt(result.value, value);
    return result;
}

BigInteger BigInteger::gcd(const BigInteger& other) const {
    BigInteger result;
    mpz_gcd(result.value, value, other.value);
    return result;
}

// Mersenne-specific optimizations
void BigInteger::set_mersenne(unsigned long exponent) {
    mpz_ui_pow_ui(value, 2, exponent);
    mpz_sub_ui(value, value, 1);
}

void BigInteger::mod_mersenne(unsigned long exponent) {
    // Optimized modular reduction for Mersenne numbers
    mpz_t high, low, mersenne;
    mpz_init(high);
    mpz_init(low);
    mpz_init(mersenne);
    
    // Compute M_p = 2^p - 1
    mpz_ui_pow_ui(mersenne, 2, exponent);
    mpz_sub_ui(mersenne, mersenne, 1);
    
    while (mpz_cmp(value, mersenne) >= 0) {
        // Split value into high and low parts
        mpz_fdiv_q_2exp(high, value, exponent);  // high = value >> p
        mpz_fdiv_r_2exp(low, value, exponent);   // low = value & ((1 << p) - 1)
        
        // value = low + high
        mpz_add(value, low, high);
    }
    
    mpz_clear(high);
    mpz_clear(low);
    mpz_clear(mersenne);
}

// Utility functions
std::string BigInteger::to_string(int base) const {
    char* str = mpz_get_str(nullptr, base, value);
    std::string result(str);
    free(str);
    return result;
}

size_t BigInteger::bit_length() const {
    return mpz_sizeinbase(value, 2);
}

bool BigInteger::is_zero() const {
    return mpz_cmp_ui(value, 0) == 0;
}

bool BigInteger::is_odd() const {
    return mpz_odd_p(value) != 0;
}

bool BigInteger::is_even() const {
    return mpz_even_p(value) != 0;
}

// Performance optimized functions
void BigInteger::square() {
    mpz_mul(value, value, value);
}

void BigInteger::double_value() {
    mpz_mul_2exp(value, value, 1);
}

void BigInteger::add_ui(unsigned long val) {
    mpz_add_ui(value, value, val);
}

void BigInteger::sub_ui(unsigned long val) {
    mpz_sub_ui(value, value, val);
}

void BigInteger::reserve_bits(size_t bits) {
    size_t limbs = (bits + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
    _mpz_realloc(value, limbs);
}

size_t BigInteger::get_memory_usage() const {
    return value->_mp_alloc * sizeof(mp_limb_t);
}

// Static utility functions
gmp_randstate_t& BigInteger::get_random_state() {
    if (!random_state_initialized) {
        initialize_random_state();
    }
    return random_state;
}

void BigInteger::initialize_random_state() {
    gmp_randinit_default(random_state);
    random_state_initialized = true;
}

void BigInteger::cleanup_random_state() {
    if (random_state_initialized) {
        gmp_randclear(random_state);
        random_state_initialized = false;
    }
}

// Memory pool management
void BigInteger::initialize_memory_pool() {
    memory_pool.reserve(POOL_SIZE);
    for (size_t i = 0; i < POOL_SIZE; ++i) {
        mpz_t* ptr = (mpz_t*)malloc(sizeof(mpz_t));
        mpz_init(*ptr);
        memory_pool.push_back(ptr);
    }
}

void BigInteger::cleanup_memory_pool() {
    for (auto ptr : memory_pool) {
        mpz_clear(*ptr);
        free(ptr);
    }
    memory_pool.clear();
    pool_index = 0;
}

mpz_t* BigInteger::allocate_from_pool() {
    if (memory_pool.empty()) {
        initialize_memory_pool();
    }
    
    if (pool_index < memory_pool.size()) {
        return memory_pool[pool_index++];
    }
    
    // Pool exhausted, allocate new
    mpz_t* ptr = (mpz_t*)malloc(sizeof(mpz_t));
    mpz_init(*ptr);
    return ptr;
}

void BigInteger::return_to_pool(mpz_t* ptr) {
    if (pool_index > 0) {
        mpz_set_ui(*ptr, 0);  // Reset value
        memory_pool[--pool_index] = ptr;
    } else {
        mpz_clear(*ptr);
        free(ptr);
    }
}

// Specialized operations for Mersenne prime calculations
namespace BigIntegerMersenne {

void square_mod_mersenne(BigInteger& result, const BigInteger& value, unsigned long exponent) {
    result = value;
    result.square();
    result.mod_mersenne(exponent);
}

void multiply_mod_mersenne(BigInteger& result, const BigInteger& a, const BigInteger& b, unsigned long exponent) {
    result = a * b;
    result.mod_mersenne(exponent);
}

void reduce_mod_mersenne(BigInteger& value, unsigned long exponent) {
    value.mod_mersenne(exponent);
}

bool is_mersenne_form(const BigInteger& value, unsigned long& exponent) {
    // Check if value is of the form 2^p - 1
    BigInteger temp = value;
    temp.add_ui(1);  // temp = value + 1
    
    // Check if temp is a power of 2
    if (temp.is_zero()) return false;
    
    size_t bits = temp.bit_length();
    if (bits == 0) return false;
    
    // Check if temp has only one bit set
    BigInteger power_of_two;
    power_of_two = BigInteger(1) << (bits - 1);
    
    if (temp == power_of_two) {
        exponent = bits - 1;
        return true;
    }
    
    return false;
}

} // namespace BigIntegerMersenne

// Stream operators
std::ostream& operator<<(std::ostream& os, const BigInteger& num) {
    os << num.to_string();
    return os;
}

std::istream& operator>>(std::istream& is, BigInteger& num) {
    std::string str;
    is >> str;
    num = BigInteger(str);
    return is;
}
