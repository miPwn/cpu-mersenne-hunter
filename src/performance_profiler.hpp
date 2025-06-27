#ifndef PERFORMANCE_PROFILER_HPP
#define PERFORMANCE_PROFILER_HPP

#include <chrono>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

class PerformanceProfiler {
public:
    struct TimingInfo {
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        double duration_ms;
        size_t call_count;
        double total_time_ms;
        double min_time_ms;
        double max_time_ms;
        double avg_time_ms;
        
        TimingInfo();
        void update(double new_duration_ms);
    };
    
    struct MemoryInfo {
        size_t peak_memory_usage;
        size_t current_memory_usage;
        size_t total_allocations;
        size_t total_deallocations;
        
        MemoryInfo();
    };
    
    struct SystemInfo {
        size_t cpu_cores;
        size_t cache_line_size;
        size_t l1_cache_size;
        size_t l2_cache_size;
        size_t l3_cache_size;
        double cpu_frequency_ghz;
        size_t total_ram_mb;
        
        SystemInfo();
        void detect_system_capabilities();
    };

private:
    static std::unique_ptr<PerformanceProfiler> instance;
    static std::mutex instance_mutex;
    
    std::unordered_map<std::string, TimingInfo> timing_data;
    std::unordered_map<std::thread::id, std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point>> active_timers;
    MemoryInfo memory_stats;
    SystemInfo system_info;
    
    mutable std::mutex data_mutex;
    bool profiling_enabled;
    
    // Performance counters
    std::unordered_map<std::string, uint64_t> counters;
    
    // Sampling profiler for continuous monitoring
    std::thread sampling_thread;
    std::atomic<bool> sampling_active{false};
    std::chrono::milliseconds sampling_interval{100};
    std::vector<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, MemoryInfo>> memory_samples;
    
    void sampling_loop();
    size_t get_current_memory_usage() const;
    
public:
    PerformanceProfiler();
    ~PerformanceProfiler();
    
    // Singleton access
    static PerformanceProfiler& get_instance();
    static void cleanup_instance();
    
    // Timer interface
    void start_timer(const std::string& name);
    void end_timer(const std::string& name);
    void reset_timer(const std::string& name);
    void reset_all_timers();
    
    // RAII timer helper
    class ScopedTimer {
    private:
        PerformanceProfiler& profiler;
        std::string timer_name;
        
    public:
        ScopedTimer(PerformanceProfiler& prof, const std::string& name);
        ~ScopedTimer();
    };
    
    // Memory tracking
    void record_memory_allocation(size_t bytes);
    void record_memory_deallocation(size_t bytes);
    void update_peak_memory();
    
    // Performance counters
    void increment_counter(const std::string& name, uint64_t value = 1);
    void set_counter(const std::string& name, uint64_t value);
    uint64_t get_counter(const std::string& name) const;
    
    // Sampling control
    void start_sampling(std::chrono::milliseconds interval = std::chrono::milliseconds(100));
    void stop_sampling();
    
    // Data access
    const TimingInfo& get_timing_info(const std::string& name) const;
    const MemoryInfo& get_memory_info() const { return memory_stats; }
    const SystemInfo& get_system_info() const { return system_info; }
    
    std::vector<std::pair<std::string, TimingInfo>> get_all_timing_data() const;
    std::vector<std::pair<std::string, uint64_t>> get_all_counters() const;
    
    // Reporting
    void print_summary(std::ostream& os = std::cout) const;
    void print_detailed_report(std::ostream& os = std::cout) const;
    void save_report_to_file(const std::string& filename) const;
    void save_csv_report(const std::string& filename) const;
    
    // JSON export for external analysis
    std::string export_json() const;
    void save_json_report(const std::string& filename) const;
    
    // Profiling control
    void enable_profiling() { profiling_enabled = true; }
    void disable_profiling() { profiling_enabled = false; }
    bool is_profiling_enabled() const { return profiling_enabled; }
    
    // Benchmarking utilities
    template<typename Func>
    double benchmark_function(const std::string& name, Func&& func, int iterations = 1);
    
    template<typename Func>
    std::vector<double> benchmark_function_samples(const std::string& name, Func&& func, int samples = 10);
    
    // Memory profiling for specific operations
    template<typename Func>
    std::pair<double, size_t> profile_memory_and_time(const std::string& name, Func&& func);
    
    // Statistical analysis
    struct Statistics {
        double mean;
        double median;
        double std_deviation;
        double min_value;
        double max_value;
        size_t sample_count;
        
        Statistics();
        Statistics(const std::vector<double>& samples);
    };
    
    Statistics analyze_timing_statistics(const std::string& timer_name) const;
    Statistics analyze_memory_usage_over_time() const;
    
    // Hardware performance counters (if available)
    void enable_hardware_counters();
    void disable_hardware_counters();
    uint64_t get_cpu_cycles() const;
    uint64_t get_cache_misses() const;
    uint64_t get_instructions_retired() const;
};

// Convenience macros for profiling
#define PROFILE_SCOPE(profiler, name) \
    PerformanceProfiler::ScopedTimer timer_##__LINE__(profiler, name)

#define PROFILE_FUNCTION(profiler) \
    PROFILE_SCOPE(profiler, __FUNCTION__)

#define PROFILE_BLOCK(profiler, name) \
    for (bool _prof_done = (profiler.start_timer(name), false); !_prof_done; \
         _prof_done = true, profiler.end_timer(name))

// Template implementations
template<typename Func>
double PerformanceProfiler::benchmark_function(const std::string& name, Func&& func, int iterations) {
    if (!profiling_enabled) return 0.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    double avg_time = duration / iterations;
    
    std::lock_guard<std::mutex> lock(data_mutex);
    timing_data[name].update(avg_time);
    
    return avg_time;
}

template<typename Func>
std::vector<double> PerformanceProfiler::benchmark_function_samples(const std::string& name, Func&& func, int samples) {
    std::vector<double> results;
    results.reserve(samples);
    
    for (int i = 0; i < samples; ++i) {
        double time = benchmark_function(name + "_sample", func, 1);
        results.push_back(time);
    }
    
    return results;
}

template<typename Func>
std::pair<double, size_t> PerformanceProfiler::profile_memory_and_time(const std::string& name, Func&& func) {
    if (!profiling_enabled) return {0.0, 0};
    
    size_t initial_memory = get_current_memory_usage();
    auto start = std::chrono::high_resolution_clock::now();
    
    func();
    
    auto end = std::chrono::high_resolution_clock::now();
    size_t final_memory = get_current_memory_usage();
    
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    size_t memory_used = (final_memory > initial_memory) ? (final_memory - initial_memory) : 0;
    
    std::lock_guard<std::mutex> lock(data_mutex);
    timing_data[name].update(duration);
    
    return {duration, memory_used};
}

#endif // PERFORMANCE_PROFILER_HPP
