#include "Log.h"
#include <stdarg.h>

static LogLevel currentLogLevel = LogLevel::DEBUG;

void log_set_level(LogLevel level) {
    currentLogLevel = level;
}

void log_print(LogLevel level, const char* format, ...){
    if (level > currentLogLevel) return;
    const char* levelStr = "";

    switch(level){
        case LogLevel::ERROR: levelStr = "[ERROR] "; break;
        case LogLevel::WARN: levelStr = "[WARN] "; break;
        case LogLevel::INFO: levelStr = "[INFO] "; break;
        case LogLevel::DEBUG: levelStr = "[DEBUG] "; break;
    }

    printf("%s", levelStr);
    
    va_list args;
    va_start(args,format);
    vprintf(format, args);
    va_end(args);

}