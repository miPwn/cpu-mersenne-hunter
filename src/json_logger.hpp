#ifndef JSON_LOGGER_HPP
#define JSON_LOGGER_HPP

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

class JSONLogger {
private:
    std::string log_dir;
    std::string results_file;
    std::string logs_file;
    
    std::string get_iso_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
        return ss.str();
    }
    
    std::string escape_json_string(const std::string& str) const {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }

public:
    JSONLogger(const std::string& log_directory = "logs") : log_dir(log_directory) {
        results_file = log_dir + "/results.ndjson";
        logs_file = log_dir + "/application.log";
        
        // Create log directory if it doesn't exist
        system(("mkdir -p " + log_dir).c_str());
    }
    
    void log_result(uint64_t exponent, bool is_prime, double duration_ms, 
                   int threads, uint64_t iterations, uint64_t fft_ops = 0) {
        std::ofstream file(results_file, std::ios::app);
        if (file.is_open()) {
            file << "{"
                 << "\"timestamp\":\"" << get_iso_timestamp() << "\","
                 << "\"exponent\":" << exponent << ","
                 << "\"isPrime\":" << (is_prime ? "true" : "false") << ","
                 << "\"duration\":" << duration_ms << ","
                 << "\"threads\":" << threads << ","
                 << "\"iterations\":" << iterations << ","
                 << "\"fftOps\":" << fft_ops << ","
                 << "\"status\":\"" << (is_prime ? "PRIME" : "COMPOSITE") << "\""
                 << "}\n";
            file.flush();
        }
    }
    
    void log_message(const std::string& level, const std::string& message, 
                    uint64_t exponent = 0) {
        std::ofstream file(logs_file, std::ios::app);
        if (file.is_open()) {
            file << "{"
                 << "\"timestamp\":\"" << get_iso_timestamp() << "\","
                 << "\"level\":\"" << level << "\","
                 << "\"message\":\"" << escape_json_string(message) << "\"";
            
            if (exponent > 0) {
                file << ",\"exponent\":" << exponent;
            }
            
            file << "}\n";
            file.flush();
        }
    }
    
    void log_info(const std::string& message, uint64_t exponent = 0) {
        log_message("info", message, exponent);
    }
    
    void log_error(const std::string& message, uint64_t exponent = 0) {
        log_message("error", message, exponent);
    }
    
    void log_debug(const std::string& message, uint64_t exponent = 0) {
        log_message("debug", message, exponent);
    }
    
    void log_warn(const std::string& message, uint64_t exponent = 0) {
        log_message("warn", message, exponent);
    }
};

#endif // JSON_LOGGER_HPP