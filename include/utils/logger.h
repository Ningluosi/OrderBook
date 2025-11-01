#pragma once
#include <mutex>
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif


namespace utils {

class Logger {
public:
    static Logger& instance();

    void setLogFile(const std::string& filename);
    void disableFileLogging();
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex mtx_;
    std::ofstream file_;
    bool toFile_ = false;

    std::string timestamp();
};

}

#define LOG_ERROR(msg) do { if (LOG_LEVEL >= 1) Logger::instance().error(msg); } while(0)
#define LOG_WARN(msg)  do { if (LOG_LEVEL >= 2)  Logger::instance().warn(msg);  } while(0)
#define LOG_INFO(msg)  do { if (LOG_LEVEL >= 3)  Logger::instance().info(msg);  } while(0)
