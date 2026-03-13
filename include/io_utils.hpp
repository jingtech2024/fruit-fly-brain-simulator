#ifndef IO_UTILS_HPP
#define IO_UTILS_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>

namespace flybrain {
namespace io {

/**
 * @brief Simple CSV parser for connectivity data
 */
class CSVReader {
public:
    explicit CSVReader(const std::string& filename);

    bool open();
    bool hasNext() const;
    std::vector<std::string> nextRow();
    void close();

private:
    std::ifstream file_;
    std::string filename_;
    std::string line_;
    bool has_more_;
};

/**
 * @brief CSV writer for output data
 */
class CSVWriter {
public:
    explicit CSVWriter(const std::string& filename);

    bool open();
    void writeRow(const std::vector<std::string>& row);
    template<typename T>
    void writeRow(const std::vector<T>& row) {
        std::vector<std::string> str_row;
        for (const auto& val : row) {
            str_row.push_back(std::to_string(val));
        }
        writeRow(str_row);
    }
    void close();

private:
    std::ofstream file_;
    std::string filename_;
};

/**
 * @brief Binary file utilities for large data
 */
class BinaryIO {
public:
    static bool writeBinary(const std::string& filename,
                           const void* data,
                           size_t size);

    static bool readBinary(const std::string& filename,
                          void* data,
                          size_t size);

    template<typename T>
    static bool writeVector(const std::string& filename,
                           const std::vector<T>& vec) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return false;

        size_t size = vec.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(vec.data()),
                   vec.size() * sizeof(T));
        return file.good();
    }

    template<typename T>
    static bool readVector(const std::string& filename,
                          std::vector<T>& vec) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;

        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        vec.resize(size);
        file.read(reinterpret_cast<char*>(vec.data()),
                  size * sizeof(T));
        return file.good();
    }
};

/**
 * @brief Progress bar for long simulations
 */
class ProgressBar {
public:
    ProgressBar(size_t total, const std::string& label = "Progress");

    void update(size_t current);
    void finish();

private:
    size_t total_;
    std::string label_;
    size_t last_printed_;
};

/**
 * @brief Logger for simulation events
 */
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void setLevel(Level level);
    static void setFile(const std::string& filename);

    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warning(const std::string& msg);
    static void error(const std::string& msg);

private:
    static Level level_;
    static std::ofstream log_file_;
    static bool use_file_;
};

/**
 * @brief Configuration file parser
 */
class ConfigParser {
public:
    static bool parse(const std::string& filename);

    template<typename T>
    static T get(const std::string& key, T default_value);

    static bool hasKey(const std::string& key);

private:
    static std::unordered_map<std::string, std::string> config_;
};

} // namespace io
} // namespace flybrain

#endif // IO_UTILS_HPP
