#include "utils/logger.h"

namespace utils {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    file_.open(filename, std::ios::app);
    toFile_ = true;
}

void Logger::disableFileLogging() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_.is_open()) file_.close();
    toFile_ = false;
}

void Logger::info(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string line = "[" + timestamp() + "][INFO] " + msg + "\n";
    if (toFile_) file_ << line; else std::cout << line;
}

void Logger::warn(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string line = "[" + timestamp() + "][WARN] " + msg + "\n";
    if (toFile_) file_ << line; else std::cout << line;
}

void Logger::error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string line = "[" + timestamp() + "][ERROR] " + msg + "\n";
    if (toFile_) file_ << line; else std::cerr << line;
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

std::string Logger::timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&t));
    return std::string(buf);
}

}