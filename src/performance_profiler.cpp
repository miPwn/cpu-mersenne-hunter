#include "performance_profiler.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>
#include <fstream>

#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#include <fstream>
#include <string>
#endif

// Static member initialization
std::unique_ptr<PerformanceProfiler> PerformanceProfiler::instance;
std::mutex PerformanceProfiler::instance_mutex;

// TimingInfo implementation
PerformanceProfiler::TimingInfo::TimingInfo() 
    : duration_ms(0.0), call_count(0), total_time_ms(0.0), 
      min_time_ms(std::numeric_limits<double>::max()), 
      max_time_ms(0.0), avg_time_ms(0.0) {}

void PerformanceProfiler::TimingInfo::update(double new_duration_ms) {
    duration_ms = new_duration_ms;
    call_count++;
    total_time_ms += new_duration_ms;
    min_time_ms = std::min(min_time_ms, new_duration_ms);
    max_time_ms = std::max(max_time_ms, new_duration_ms);
    avg_time_ms = total_time_ms / call_count;
}

// MemoryInfo implementation
PerformanceProfiler::MemoryInfo::MemoryInfo()
    : peak_memory_usage(0), current_memory_usage(0), 
      total_allocations(0), total_deallocations(0) {}

// SystemInfo implementation
PerformanceProfiler::SystemInfo::SystemInfo()
    : cpu_cores(1), cache_line_size(64), l1_cache_size(32768), 
      l2_cache_size(262144), l3_cache_size(8388608), 
      cpu_frequency_ghz(2.0), total_ram_mb(8192) {
    detect_system_capabilities();
}

void PerformanceProfiler::SystemInfo::detect_system_capabilities() {
#ifdef __linux__
    // Detect CPU cores
    cpu_cores = std::thread::hardware_concurrency();
    
    // Read CPU info from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                cpu_frequency_ghz = std::stod(line.substr(pos + 1)) / 1000.0;
                break;
            }
        }
    }
    
    // Read memory info from /proc/meminfo
    std::ifstream meminfo("/proc/meminfo");
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key, value, unit;
            iss >> key >> value >> unit;
            total_ram_mb = std::stoull(value) / 1024;  // Convert from KB to MB
            break;
        }
    }
    
    // Cache sizes (simplified detection)
    std::ifstream cache_info("/sys/devices/system/cpu/cpu0/cache/index0/size");
    if (cache_info) {
        std::string cache_size_str;
        cache_info >> cache_size_str;
        if (!cache_size_str.empty()) {
            l1_cache_size = std::stoull(cache_size_str) * 1024;  // Assume KB
        }
    }
#endif
}

// PerformanceProfiler implementation
PerformanceProfiler::PerformanceProfiler() : profiling_enabled(true) {
    memory_stats = MemoryInfo();
    system_info = SystemInfo();
}

PerformanceProfiler::~PerformanceProfiler() {
    stop_sampling();
}

PerformanceProfiler& PerformanceProfiler::get_instance() {
    std::lock_guard<std::mutex> lock(instance_mutex);
    if (!instance) {
        instance = std::make_unique<PerformanceProfiler>();
    }
    return *instance;
}

void PerformanceProfiler::cleanup_instance() {
    std::lock_guard<std::mutex> lock(instance_mutex);
    instance.reset();
}

void PerformanceProfiler::start_timer(const std::string& name) {
    if (!profiling_enabled) return;
    
    std::lock_guard<std::mutex> lock(data_mutex);
    auto thread_id = std::this_thread::get_id();
    active_timers[thread_id][name] = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::end_timer(const std::string& name) {
    if (!profiling_enabled) return;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(data_mutex);
    auto thread_id = std::this_thread::get_id();
    
    auto it = active_timers[thread_id].find(name);
    if (it != active_timers[thread_id].end()) {
        auto duration = std::chrono::duration<double, std::milli>(end_time - it->second).count();
        timing_data[name].update(duration);
        active_timers[thread_id].erase(it);
    }
}

void PerformanceProfiler::reset_timer(const std::string& name) {
    std::lock_guard<std::mutex> lock(data_mutex);
    timing_data.erase(name);
}

void PerformanceProfiler::reset_all_timers() {
    std::lock_guard<std::mutex> lock(data_mutex);
    timing_data.clear();
    active_timers.clear();
}

// ScopedTimer implementation
PerformanceProfiler::ScopedTimer::ScopedTimer(PerformanceProfiler& prof, const std::string& name)
    : profiler(prof), timer_name(name) {
    profiler.start_timer(timer_name);
}

PerformanceProfiler::ScopedTimer::~ScopedTimer() {
    profiler.end_timer(timer_name);
}

void PerformanceProfiler::record_memory_allocation(size_t bytes) {
    std::lock_guard<std::mutex> lock(data_mutex);
    memory_stats.current_memory_usage += bytes;
    memory_stats.total_allocations += bytes;
    memory_stats.peak_memory_usage = std::max(memory_stats.peak_memory_usage, 
                                             memory_stats.current_memory_usage);
}

void PerformanceProfiler::record_memory_deallocation(size_t bytes) {
    std::lock_guard<std::mutex> lock(data_mutex);
    memory_stats.current_memory_usage = (memory_stats.current_memory_usage > bytes) 
                                       ? memory_stats.current_memory_usage - bytes : 0;
    memory_stats.total_deallocations += bytes;
}

void PerformanceProfiler::update_peak_memory() {
    std::lock_guard<std::mutex> lock(data_mutex);
    size_t current = get_current_memory_usage();
    memory_stats.peak_memory_usage = std::max(memory_stats.peak_memory_usage, current);
}

size_t PerformanceProfiler::get_current_memory_usage() {
#ifdef __linux__
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.find("VmRSS:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key, value, unit;
            iss >> key >> value >> unit;
            return std::stoull(value) * 1024;  // Convert from KB to bytes
        }
    }
#endif
    return memory_stats.current_memory_usage;
}

void PerformanceProfiler::increment_counter(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(data_mutex);
    counters[name] += value;
}

void PerformanceProfiler::set_counter(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(data_mutex);
    counters[name] = value;
}

uint64_t PerformanceProfiler::get_counter(const std::string& name) const {
    std::lock_guard<std::mutex> lock(data_mutex);
    auto it = counters.find(name);
    return (it != counters.end()) ? it->second : 0;
}

void PerformanceProfiler::start_sampling(std::chrono::milliseconds interval) {
    if (sampling_active.load()) {
        stop_sampling();
    }
    
    sampling_interval = interval;
    sampling_active.store(true);
    sampling_thread = std::thread(&PerformanceProfiler::sampling_loop, this);
}

void PerformanceProfiler::stop_sampling() {
    if (sampling_active.load()) {
        sampling_active.store(false);
        if (sampling_thread.joinable()) {
            sampling_thread.join();
        }
    }
}

void PerformanceProfiler::sampling_loop() {
    while (sampling_active.load()) {
        auto now = std::chrono::high_resolution_clock::now();
        MemoryInfo current_memory;
        current_memory.current_memory_usage = get_current_memory_usage();
        
        {
            std::lock_guard<std::mutex> lock(data_mutex);
            memory_samples.emplace_back(now, current_memory);
        }
        
        std::this_thread::sleep_for(sampling_interval);
    }
}

const PerformanceProfiler::TimingInfo& PerformanceProfiler::get_timing_info(const std::string& name) const {
    std::lock_guard<std::mutex> lock(data_mutex);
    static TimingInfo empty_info;
    auto it = timing_data.find(name);
    return (it != timing_data.end()) ? it->second : empty_info;
}

std::vector<std::pair<std::string, PerformanceProfiler::TimingInfo>> PerformanceProfiler::get_all_timing_data() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    std::vector<std::pair<std::string, TimingInfo>> result;
    result.reserve(timing_data.size());
    
    for (const auto& pair : timing_data) {
        result.emplace_back(pair.first, pair.second);
    }
    
    // Sort by total time (descending)
    std::sort(result.begin(), result.end(), 
              [](const auto& a, const auto& b) { 
                  return a.second.total_time_ms > b.second.total_time_ms; 
              });
    
    return result;
}

std::vector<std::pair<std::string, uint64_t>> PerformanceProfiler::get_all_counters() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    std::vector<std::pair<std::string, uint64_t>> result;
    result.reserve(counters.size());
    
    for (const auto& pair : counters) {
        result.emplace_back(pair.first, pair.second);
    }
    
    return result;
}

void PerformanceProfiler::print_summary(std::ostream& os) const {
    auto timing_data = get_all_timing_data();
    
    os << "\n=== Performance Summary ===" << std::endl;
    os << std::fixed << std::setprecision(3);
    
    if (!timing_data.empty()) {
        os << "\nTop 10 Time Consumers:" << std::endl;
        os << std::setw(30) << "Function" 
           << std::setw(12) << "Total (ms)" 
           << std::setw(12) << "Calls" 
           << std::setw(12) << "Avg (ms)" << std::endl;
        os << std::string(66, '-') << std::endl;
        
        for (size_t i = 0; i < std::min(timing_data.size(), size_t(10)); ++i) {
            const auto& [name, info] = timing_data[i];
            os << std::setw(30) << name
               << std::setw(12) << info.total_time_ms
               << std::setw(12) << info.call_count
               << std::setw(12) << info.avg_time_ms << std::endl;
        }
    }
    
    os << "\nMemory Usage:" << std::endl;
    os << "  Peak: " << (memory_stats.peak_memory_usage / 1024 / 1024) << " MB" << std::endl;
    os << "  Current: " << (get_current_memory_usage() / 1024 / 1024) << " MB" << std::endl;
    
    auto counter_data = get_all_counters();
    if (!counter_data.empty()) {
        os << "\nCounters:" << std::endl;
        for (const auto& [name, value] : counter_data) {
            os << "  " << name << ": " << value << std::endl;
        }
    }
    
    os << "\nSystem Info:" << std::endl;
    os << "  CPU Cores: " << system_info.cpu_cores << std::endl;
    os << "  CPU Frequency: " << system_info.cpu_frequency_ghz << " GHz" << std::endl;
    os << "  Total RAM: " << system_info.total_ram_mb << " MB" << std::endl;
}

void PerformanceProfiler::print_detailed_report(std::ostream& os) const {
    auto timing_data = get_all_timing_data();
    
    os << "\n=== Detailed Performance Report ===" << std::endl;
    os << std::fixed << std::setprecision(6);
    
    for (const auto& [name, info] : timing_data) {
        os << "\n" << name << ":" << std::endl;
        os << "  Calls: " << info.call_count << std::endl;
        os << "  Total Time: " << info.total_time_ms << " ms" << std::endl;
        os << "  Average Time: " << info.avg_time_ms << " ms" << std::endl;
        os << "  Min Time: " << info.min_time_ms << " ms" << std::endl;
        os << "  Max Time: " << info.max_time_ms << " ms" << std::endl;
    }
}

void PerformanceProfiler::save_report_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        print_detailed_report(file);
        file.close();
    }
}

void PerformanceProfiler::save_csv_report(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    auto timing_data = get_all_timing_data();
    
    file << "Function,Calls,Total_Time_ms,Average_Time_ms,Min_Time_ms,Max_Time_ms\n";
    
    for (const auto& [name, info] : timing_data) {
        file << name << ","
             << info.call_count << ","
             << info.total_time_ms << ","
             << info.avg_time_ms << ","
             << info.min_time_ms << ","
             << info.max_time_ms << "\n";
    }
    
    file.close();
}

std::string PerformanceProfiler::export_json() const {
    std::ostringstream json;
    auto timing_data = get_all_timing_data();
    auto counter_data = get_all_counters();
    
    json << "{\n";
    json << "  \"timing_data\": [\n";
    
    for (size_t i = 0; i < timing_data.size(); ++i) {
        const auto& [name, info] = timing_data[i];
        json << "    {\n";
        json << "      \"name\": \"" << name << "\",\n";
        json << "      \"calls\": " << info.call_count << ",\n";
        json << "      \"total_time_ms\": " << info.total_time_ms << ",\n";
        json << "      \"average_time_ms\": " << info.avg_time_ms << ",\n";
        json << "      \"min_time_ms\": " << info.min_time_ms << ",\n";
        json << "      \"max_time_ms\": " << info.max_time_ms << "\n";
        json << "    }";
        if (i < timing_data.size() - 1) json << ",";
        json << "\n";
    }
    
    json << "  ],\n";
    json << "  \"memory_info\": {\n";
    json << "    \"peak_memory_usage\": " << memory_stats.peak_memory_usage << ",\n";
    json << "    \"current_memory_usage\": " << get_current_memory_usage() << "\n";
    json << "  },\n";
    json << "  \"system_info\": {\n";
    json << "    \"cpu_cores\": " << system_info.cpu_cores << ",\n";
    json << "    \"cpu_frequency_ghz\": " << system_info.cpu_frequency_ghz << ",\n";
    json << "    \"total_ram_mb\": " << system_info.total_ram_mb << "\n";
    json << "  }\n";
    json << "}\n";
    
    return json.str();
}

void PerformanceProfiler::save_json_report(const std::string& filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << export_json();
        file.close();
    }
}

// Statistics implementation
PerformanceProfiler::Statistics::Statistics()
    : mean(0.0), median(0.0), std_deviation(0.0), 
      min_value(0.0), max_value(0.0), sample_count(0) {}

PerformanceProfiler::Statistics::Statistics(const std::vector<double>& samples) {
    if (samples.empty()) {
        *this = Statistics();
        return;
    }
    
    sample_count = samples.size();
    
    // Calculate mean
    mean = std::accumulate(samples.begin(), samples.end(), 0.0) / sample_count;
    
    // Calculate min and max
    auto minmax = std::minmax_element(samples.begin(), samples.end());
    min_value = *minmax.first;
    max_value = *minmax.second;
    
    // Calculate median
    std::vector<double> sorted_samples = samples;
    std::sort(sorted_samples.begin(), sorted_samples.end());
    
    if (sample_count % 2 == 0) {
        median = (sorted_samples[sample_count/2 - 1] + sorted_samples[sample_count/2]) / 2.0;
    } else {
        median = sorted_samples[sample_count/2];
    }
    
    // Calculate standard deviation
    double variance = 0.0;
    for (double sample : samples) {
        double diff = sample - mean;
        variance += diff * diff;
    }
    variance /= sample_count;
    std_deviation = std::sqrt(variance);
}

PerformanceProfiler::Statistics PerformanceProfiler::analyze_timing_statistics(const std::string& timer_name) const {
    // This would require storing individual timing samples, not just aggregated data
    // For now, return basic statistics from the available data
    auto info = get_timing_info(timer_name);
    
    if (info.call_count == 0) {
        return Statistics();
    }
    
    // Create a simple statistics object with available data
    Statistics stats;
    stats.mean = info.avg_time_ms;
    stats.min_value = info.min_time_ms;
    stats.max_value = info.max_time_ms;
    stats.sample_count = info.call_count;
    
    return stats;
}

PerformanceProfiler::Statistics PerformanceProfiler::analyze_memory_usage_over_time() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    
    if (memory_samples.empty()) {
        return Statistics();
    }
    
    std::vector<double> memory_values;
    memory_values.reserve(memory_samples.size());
    
    for (const auto& sample : memory_samples) {
        memory_values.push_back(static_cast<double>(sample.second.current_memory_usage));
    }
    
    return Statistics(memory_values);
}

void PerformanceProfiler::enable_hardware_counters() {
    // Implementation would depend on available hardware performance monitoring
    // This is a placeholder for future implementation
}

void PerformanceProfiler::disable_hardware_counters() {
    // Placeholder
}

uint64_t PerformanceProfiler::get_cpu_cycles() const {
    // Placeholder - would use platform-specific performance counters
    return 0;
}

uint64_t PerformanceProfiler::get_cache_misses() const {
    // Placeholder
    return 0;
}

uint64_t PerformanceProfiler::get_instructions_retired() const {
    // Placeholder
    return 0;
}
