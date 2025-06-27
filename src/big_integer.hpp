#ifndef BIG_INTEGER_HPP
#define BIG_INTEGER_HPP

#include <gmp.h>
#include <vector>
#include <memory>
#include <string>

class BigInteger {
private:
    mpz_t value;
    static gmp_randstate_t random_state;
    static bool random_state_initialized;
    
    // Memory pool for frequent allocations
    static constexpr size_t POOL_SIZE = 1024;
    static std::vector<mpz_t*> memory_pool;
    static size_t pool_index;
    
public:
    // Constructors
    BigInteger();
    BigInteger(long long val);
    BigInteger(const std::string& str, int base = 10);
    BigInteger(const BigInteger& other);
    BigInteger(BigInteger&& other) noexcept;
    
    // Destructor
    ~BigInteger();
    
    // Assignment operators
    BigInteger& operator=(const BigInteger& other);
    BigInteger& operator=(BigInteger&& other) noexcept;
    BigInteger& operator=(long long val);
    
    // Arithmetic operators
    BigInteger operator+(const BigInteger& other) const;
    BigInteger operator-(const BigInteger& other) const;
    BigInteger operator*(const BigInteger& other) const;
    BigInteger operator/(const BigInteger& other) const;
    BigInteger operator%(const BigInteger& other) const;
    
    BigInteger& operator+=(const BigInteger& other);
    BigInteger& operator-=(const BigInteger& other);
    BigInteger& operator*=(const BigInteger& other);
    BigInteger& operator/=(const BigInteger& other);
    BigInteger& operator%=(const BigInteger& other);
    
    // Comparison operators
    bool operator==(const BigInteger& other) const;
    bool operator!=(const BigInteger& other) const;
    bool operator<(const BigInteger& other) const;
    bool operator<=(const BigInteger& other) const;
    bool operator>(const BigInteger& other) const;
    bool operator>=(const BigInteger& other) const;
    
    // Bitwise operations
    BigInteger operator<<(unsigned long shift) const;
    BigInteger operator>>(unsigned long shift) const;
    BigInteger& operator<<=(unsigned long shift);
    BigInteger& operator>>=(unsigned long shift);
    
    // Mathematical functions
    BigInteger pow(unsigned long exponent) const;
    BigInteger mod_pow(const BigInteger& exponent, const BigInteger& modulus) const;
    BigInteger sqrt() const;
    BigInteger gcd(const BigInteger& other) const;
    
    // Mersenne-specific optimizations
    void set_mersenne(unsigned long exponent);  // Set to 2^exponent - 1
    void mod_mersenne(unsigned long exponent);  // Reduce modulo 2^exponent - 1
    
    // Utility functions
    std::string to_string(int base = 10) const;
    size_t bit_length() const;
    bool is_zero() const;
    bool is_odd() const;
    bool is_even() const;
    
    // Performance optimized functions
    void square();  // In-place squaring
    void double_value();  // In-place doubling
    void add_ui(unsigned long val);  // Add unsigned long
    void sub_ui(unsigned long val);  // Subtract unsigned long
    
    // Memory management
    void reserve_bits(size_t bits);  // Pre-allocate memory
    size_t get_memory_usage() const;  // Get current memory usage
    
    // Access to underlying GMP type (for interfacing with GMP functions)
    mpz_t& get_mpz() { return value; }
    const mpz_t& get_mpz() const { return value; }
    
    // Static utility functions
    static gmp_randstate_t& get_random_state();
    static void initialize_random_state();
    static void cleanup_random_state();
    
    // Memory pool management
    static void initialize_memory_pool();
    static void cleanup_memory_pool();
    static mpz_t* allocate_from_pool();
    static void return_to_pool(mpz_t* ptr);
};

// Specialized operations for Mersenne prime calculations
namespace BigIntegerMersenne {
    // Fast squaring modulo Mersenne numbers
    void square_mod_mersenne(BigInteger& result, const BigInteger& value, unsigned long exponent);
    
    // Fast multiplication modulo Mersenne numbers
    void multiply_mod_mersenne(BigInteger& result, const BigInteger& a, const BigInteger& b, unsigned long exponent);
    
    // Optimized modular reduction for Mersenne numbers
    void reduce_mod_mersenne(BigInteger& value, unsigned long exponent);
    
    // Check if number is a Mersenne number
    bool is_mersenne_form(const BigInteger& value, unsigned long& exponent);
}

// Stream operators
std::ostream& operator<<(std::ostream& os, const BigInteger& num);
std::istream& operator>>(std::istream& is, BigInteger& num);

#endif // BIG_INTEGER_HPP
