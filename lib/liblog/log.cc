#include <cinttypes>

#include <chrono>
#include <iomanip>


#include "log.hh"


void Log::log_format(const char* type, const char* proc, const char* file, const int line,
                     const char* func, const char* fmt, ...) {
    const auto now = std::chrono::system_clock::now();
    const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;
    const auto rawtime = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&rawtime), "%F %T");

    fprintf(stderr, "%s.%06" PRId64 " %s: [ %s %s:%d %s ] ", ss.str().c_str(), usec, proc, type, file, line, func);
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
}