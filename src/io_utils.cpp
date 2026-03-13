#include "io_utils.hpp"
#include <iostream>
#include <cstring>

namespace flybrain {
namespace io {

// CSVReader implementation
CSVReader::CSVReader(const std::string& filename)
    : file_(filename)
    , filename_(filename)
    , has_more_(false)
{
}

bool CSVReader::open() {
    if (!file_.is_open()) {
        file_.open(filename_, std::ios::in);
    }
    has_more_ = file_.good() && !file_.eof();
    return file_.is_open();
}

bool CSVReader::hasNext() const {
    return has_more_;
}

std::vector<std::string> CSVReader::nextRow() {
    std::vector<std::string> row;

    if (!has_more_) return row;

    std::string line;
    if (std::getline(file_, line)) {
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        has_more_ = !file_.eof();
    } else {
        has_more_ = false;
    }

    return row;
}

void CSVReader::close() {
    if (file_.is_open()) {
        file_.close();
    }
    has_more_ = false;
}

// CSVWriter implementation
CSVWriter::CSVWriter(const std::string& filename)
    : file_(filename)
    , filename_(filename)
{
}

bool CSVWriter::open() {
    if (!file_.is_open()) {
        file_.open(filename_, std::ios::out | std::ios::trunc);
    }
    return file_.is_open();
}

void CSVWriter::writeRow(const std::vector<std::string>& row) {
    for (size_t i = 0; i < row.size(); ++i) {
        file_ << row[i];
        if (i < row.size() - 1) {
            file_ << ",";
        }
    }
    file_ << "\n";
}

void CSVWriter::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

// BinaryIO implementation
bool BinaryIO::writeBinary(const std::string& filename,
                          const void* data,
                          size_t size) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    file.write(static_cast<const char*>(data), size);
    return file.good();
}

bool BinaryIO::readBinary(const std::string& filename,
                         void* data,
                         size_t size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    file.read(static_cast<char*>(data), size);
    return file.good();
}

// ProgressBar implementation
ProgressBar::ProgressBar(size_t total, const std::string& label)
    : total_(total)
    , label_(label)
    , last_printed_(SIZE_MAX)
{
}

void ProgressBar::update(size_t current) {
    size_t percent = (current * 100) / total_;

    // Only print if percent changed
    if (percent != last_printed_) {
        std::cout << "\r" << label_ << ": [";
        size_t filled = percent / 2;
        for (size_t i = 0; i < 50; ++i) {
            if (i < filled) {
                std::cout << "=";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "] " << percent << "%";
        std::cout.flush();
        last_printed_ = percent;
    }
}

void ProgressBar::finish() {
    std::cout << std::endl;
}

// Logger implementation
Logger::Level Logger::level_ = Logger::Level::INFO;
std::ofstream Logger::log_file_;
bool Logger::use_file_ = false;

void Logger::setLevel(Level level) {
    level_ = level;
}

void Logger::setFile(const std::string& filename) {
    if (log_file_.is_open()) {
        log_file_.close();
    }
    log_file_.open(filename, std::ios::app);
    use_file_ = log_file_.is_open();
}

void Logger::debug(const std::string& msg) {
    if (level_ <= Level::DEBUG) {
        std::cout << "[DEBUG] " << msg << std::endl;
        if (use_file_) log_file_ << "[DEBUG] " << msg << std::endl;
    }
}

void Logger::info(const std::string& msg) {
    if (level_ <= Level::INFO) {
        std::cout << "[INFO] " << msg << std::endl;
        if (use_file_) log_file_ << "[INFO] " << msg << std::endl;
    }
}

void Logger::warning(const std::string& msg) {
    if (level_ <= Level::WARNING) {
        std::cout << "[WARN] " << msg << std::endl;
        if (use_file_) log_file_ << "[WARN] " << msg << std::endl;
    }
}

void Logger::error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
    if (use_file_) log_file_ << "[ERROR] " << msg << std::endl;
}

// ConfigParser implementation
std::unordered_map<std::string, std::string> ConfigParser::config_;

bool ConfigParser::parse(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            config_[key] = value;
        }
    }

    return true;
}

bool ConfigParser::hasKey(const std::string& key) {
    return config_.find(key) != config_.end();
}

} // namespace io
} // namespace flybrain
