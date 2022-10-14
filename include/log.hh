#pragma once

#include <cstdio>
#include <cstdarg>


#define LOG_WRAPPER(type, fmt, ...) \
    Log::log_format(type, __MODULE__, __FILENAME__, __LINE__, __func__, fmt __VA_OPT__(,) __VA_ARGS__);

#define LOG_ERROR(fmt, ...)     LOG_WRAPPER("ERROR", fmt, __VA_ARGS__);
#define LOG_WARNING(fmt, ...)   LOG_WRAPPER("WARNING", fmt, __VA_ARGS__);
#define LOG_INFO(fmt, ...)      LOG_WRAPPER("INFO", fmt, __VA_ARGS__);
#define LOG_TRACE(fmt, ...)     LOG_WRAPPER("TRACE", fmt, __VA_ARGS__);


namespace Log {
    void log_format(const char* type, const char* proc, const char* file, const int line,
                    const char* func, const char* fmt, ...);
}
