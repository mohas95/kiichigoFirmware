#ifndef Log_H
#define Log_H

#include <stdio.h>

enum class LogLevel {
    ERROR,
    WARN,
    INFO,
    DEBUG
};

void log_print(LogLevel level, const char* format, ...);
void log_set_level(LogLevel level);

#define LOG_ERROR(...) log_print(LogLevel::ERROR, __VA_ARGS__)
#define LOG_WARN(...)  log_print(LogLevel::WARN,  __VA_ARGS__)
#define LOG_INFO(...)  log_print(LogLevel::INFO,  __VA_ARGS__)
#define LOG_DEBUG(...) log_print(LogLevel::DEBUG, __VA_ARGS__)

#endif