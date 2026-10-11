#include "core/Logger.h"

#include <iostream>

namespace cnode {

void Logger::log(LogLevel level, std::string_view message) {
    const char* label = "INFO";
    switch (level) {
    case LogLevel::Info:    label = "INFO"; break;
    case LogLevel::Warning: label = "WARN"; break;
    case LogLevel::Error:   label = "ERROR"; break;
    }
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << "[CNode/" << label << "] " << message << '\n';
}

void Logger::info(std::string_view message)    { log(LogLevel::Info, message); }
void Logger::warning(std::string_view message) { log(LogLevel::Warning, message); }
void Logger::error(std::string_view message)   { log(LogLevel::Error, message); }

} // namespace cnode
