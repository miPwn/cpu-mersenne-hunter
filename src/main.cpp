#include "mersenne_prime.hpp"
#include "performance_profiler.hpp"
#include "json_logger.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <cstdlib>
#include <getopt.h>

struct ProgramOptions {
    uint64_t start_exponent = 2;
    uint64_t end_exponent = 1000;
    uint64_t single_exponent = 0;
    int num_threads = 0;
    bool range_mode = false;
    bool benchmark_mode = false;
    bool verbose = false;
    bool save_profile = false;
    std::string profile_filename = "mersenne_profile.json";
    std::string output_filename;
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -p, --prime EXPONENT     Test if 2^EXPONENT - 1 is prime\n";
    std::cout << "  -r, --range START END    Test range of exponents [START, END]\n";
    std::cout << "  -t, --threads NUM        Number of threads (0 = auto)\n";
    std::cout << "  -b, --benchmark          Run benchmarking tests\n";
    std::cout << "  -v, --verbose            Verbose output\n";
    std::cout << "  -s, --save-profile FILE  Save performance profile to file\n";
    std::cout << "  -o, --output FILE        Save results to file\n";
    std::cout << "  -h, --help               Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " -p 127         # Test M127\n";
    std::cout << "  " << program_name << " -r 100 1000    # Test M_p for p in [100, 1000]\n";
    std::cout << "  " << program_name << " -b             # Run benchmarks\n";
}

ProgramOptions parse_arguments(int argc, char* argv[]) {
    ProgramOptions options;
    
    static struct option long_options[] = {
        {"prime", required_argument, 0, 'p'},
        {"range", required_argument, 0, 'r'},
        {"threads", required_argument, 0, 't'},
        {"benchmark", no_argument, 0, 'b'},
        {"verbose", no_argument, 0, 'v'},
        {"save-profile", required_argument, 0, 's'},
        {"output", required_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "p:r:t:bvs:o:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                options.single_exponent = std::stoull(optarg);
                break;
            case 'r':
                options.range_mode = true;
                options.start_exponent = std::stoull(optarg);
                if (optind < argc && argv[optind][0] != '-') {
                    options.end_exponent = std::stoull(argv[optind]);
                    optind++;
                } else {
                    std::cerr << "Error: Range mode requires both start and end values\n";
                    exit(1);
                }
                break;
            case 't':
                options.num_threads = std::stoi(optarg);
                break;
            case 'b':
                options.benchmark_mode = true;
                break;
            case 'v':
                options.verbose = true;
                break;
            case 's':
                options.save_profile = true;
                options.profile_filename = optarg;
                break;
            case 'o':
                options.output_filename = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
                break;
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }
    
    // Set default number of threads if not specified
    if (options.num_threads == 0) {
        options.num_threads = std::thread::hardware_concurrency();
    }
    
    return options;
}

void run_single_test(const ProgramOptions& options) {
    auto& profiler = PerformanceProfiler::get_instance();
    MersennePrime mp(profiler);
    
    uint64_t exponent = options.single_exponent;
    
    if (options.verbose) {
        std::cout << "Testing M" << exponent << " = 2^" << exponent << " - 1\n";
        std::cout << "Using " << options.num_threads << " threads\n";
        std::cout << "Estimated memory usage: " 
                  << (MersennePrime::estimate_memory_usage(exponent) / 1024 / 1024) 
                  << " MB\n";
        std::cout << "Estimated computation time: " 
                  << (MersennePrime::estimate_computation_time(exponent) / 1000.0) 
                  << " seconds\n\n";
    }
    
    bool is_prime = mp.is_mersenne_prime(exponent, options.num_threads);
    
    std::cout << "M" << exponent << " = 2^" << exponent << " - 1 is ";
    std::cout << (is_prime ? "PRIME" : "COMPOSITE") << std::endl;
    
    if (options.verbose) {
        std::cout << "\nPerformance Statistics:\n";
        std::cout << "Iterations completed: " << mp.get_iterations_completed() << std::endl;
        std::cout << "FFT operations: " << mp.get_fft_operations() << std::endl;
        
        profiler.print_summary();
    }
}

void run_range_test(const ProgramOptions& options) {
    auto& profiler = PerformanceProfiler::get_instance();
    MersennePrime mp(profiler);
    
    if (options.verbose) {
        std::cout << "Searching for Mersenne primes in range [" 
                  << options.start_exponent << ", " << options.end_exponent << "]\n";
        std::cout << "Using " << options.num_threads << " threads\n\n";
    }
    
    auto primes = mp.find_mersenne_primes_range(options.start_exponent, 
                                               options.end_exponent, 
                                               options.num_threads);
    
    std::cout << "Found " << primes.size() << " Mersenne primes:\n";
    for (uint64_t p : primes) {
        std::cout << "M" << p << " = 2^" << p << " - 1\n";
    }
    
    if (options.verbose) {
        profiler.print_summary();
    }
    
    // Save results if requested
    if (!options.output_filename.empty()) {
        std::ofstream output(options.output_filename);
        output << "# Mersenne primes found in range [" 
               << options.start_exponent << ", " << options.end_exponent << "]\n";
        for (uint64_t p : primes) {
            output << p << "\n";
        }
        output.close();
        std::cout << "\nResults saved to " << options.output_filename << std::endl;
    }
}

void run_benchmarks(const ProgramOptions& options) {
    auto& profiler = PerformanceProfiler::get_instance();
    MersennePrime mp(profiler);
    
    std::cout << "Running Mersenne Prime Performance Benchmarks\n";
    std::cout << "============================================\n\n";
    
    // Test different exponent sizes
    std::vector<uint64_t> test_exponents = {127, 521, 607, 1279, 2203, 3217};
    
    for (uint64_t exponent : test_exponents) {
        if (!MersennePrime::is_prime(exponent)) continue;
        
        std::cout << "Benchmarking M" << exponent << ":\n";
        
        // Memory usage estimate
        uint64_t memory_mb = MersennePrime::estimate_memory_usage(exponent) / 1024 / 1024;
        std::cout << "  Estimated memory: " << memory_mb << " MB\n";
        
        // Time the computation
        auto start = std::chrono::high_resolution_clock::now();
        bool is_prime = mp.is_mersenne_prime(exponent, options.num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration<double>(end - start).count();
        
        std::cout << "  Result: " << (is_prime ? "PRIME" : "COMPOSITE") << "\n";
        std::cout << "  Time: " << duration << " seconds\n";
        std::cout << "  Iterations: " << mp.get_iterations_completed() << "\n";
        std::cout << "  FFT ops: " << mp.get_fft_operations() << "\n\n";
        
        // Reset counters for next test
        profiler.reset_all_timers();
    }
    
    // Thread scaling benchmark
    std::cout << "Thread Scaling Benchmark (M607):\n";
    std::vector<int> thread_counts = {1, 2, 4, 8, options.num_threads};
    
    for (int threads : thread_counts) {
        if (threads > options.num_threads) continue;
        
        auto start = std::chrono::high_resolution_clock::now();
        mp.is_mersenne_prime(607, threads);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration<double>(end - start).count();
        std::cout << "  " << threads << " threads: " << duration << " seconds\n";
    }
    
    std::cout << "\nBenchmark completed.\n";
    profiler.print_summary();
}

int main(int argc, char* argv[]) {
    try {
        ProgramOptions options = parse_arguments(argc, argv);
        
        // Initialize performance profiler
        auto& profiler = PerformanceProfiler::get_instance();
        if (options.verbose) {
            profiler.start_sampling(std::chrono::milliseconds(100));
        }
        
        std::cout << "Mersenne Prime Calculator - High Performance Edition\n";
        std::cout << "====================================================\n\n";
        
        // Display system information
        if (options.verbose) {
            const auto& sys_info = profiler.get_system_info();
            std::cout << "System Information:\n";
            std::cout << "  CPU Cores: " << sys_info.cpu_cores << "\n";
            std::cout << "  CPU Frequency: " << sys_info.cpu_frequency_ghz << " GHz\n";
            std::cout << "  Total RAM: " << sys_info.total_ram_mb << " MB\n";
            std::cout << "  Cache Line Size: " << sys_info.cache_line_size << " bytes\n\n";
        }
        
        // Execute requested operation
        if (options.benchmark_mode) {
            run_benchmarks(options);
        } else if (options.range_mode) {
            run_range_test(options);
        } else if (options.single_exponent > 0) {
            run_single_test(options);
        } else {
            std::cout << "No operation specified. Use -h for help.\n";
            return 1;
        }
        
        // Save performance profile if requested
        if (options.save_profile) {
            profiler.save_json_report(options.profile_filename);
            std::cout << "\nPerformance profile saved to " 
                      << options.profile_filename << std::endl;
        }
        
        if (options.verbose) {
            profiler.stop_sampling();
        }
        
        // Cleanup
        PerformanceProfiler::cleanup_instance();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
