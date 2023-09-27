#pragma once
#include <core/fwd.h>
#include <spdlog/spdlog.h>

#include <ostream>
#include <sstream>

namespace Color {
enum Code {
    FG_RED     = 31,
    FG_GREEN   = 32,
    FG_BLUE    = 34,
    FG_DEFAULT = 39,
    BG_RED     = 41,
    BG_GREEN   = 42,
    BG_BLUE    = 44,
    BG_DEFAULT = 49
};
class Modifier {
    Code code;

public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream &operator<<(std::ostream &os, const Modifier &mod) {
        return os << "\033[" << mod.code << "m";
    }
};
} // namespace Color

#include <string.h>

#include <iostream>
#include <memory>
#include <variant>

#include "spdlog/spdlog.h"

template <typename T>
std::string to_string(const T &value) {
    if constexpr (std::is_convertible_v<T, std::string>) {
        return value;
    } else {
        return std::to_string(value);
    }
}

struct Logger {
    using Type = std::variant<int, float, double, std::string>;

    struct Entry {
        Entry(const std::string &msg) : msg(msg), data(msg) {}

        template <typename T>
        Entry(const T &msg) : msg(to_string(msg)), data(msg) {}

        template <typename T>
        Entry(const std::string &key, const T &msg)
            : key(key), msg(to_string(msg)), data(msg) {}

        std::string key;
        std::string msg;
        Type        data;
    };

    static void    static_init();
    static void    set_level(spdlog::level::level_enum level);
    static Logger &get();

    static void info(Entry entry);
    static void warn(Entry entry);
    static void debug(Entry entry);
    static void error(Entry entry);

    template <typename... Args>
    static void info(const std::string &msg, Args... args) {
        m_logger->info(msg, args...);
    }

    template <typename... Args>
    static void debug(const std::string &msg, Args... args) {
        m_logger->debug(msg, args...);
    }

    template <typename... Args>
    static void warn(const std::string &msg, Args... args) {
        m_logger->warn(msg, args...);
    }

    template <typename... Args>
    static void error(const std::string &msg, Args... args) {
        m_logger->error(msg, args...);
    }

    static ref<spdlog::logger> m_logger;
    static spdlog::sink_ptr    m_stdout_sink;
    static spdlog::sink_ptr    m_file_sink;
    static std::vector<Entry>  m_logs;
};

#define PSDR_INFO(...)  SPDLOG_LOGGER_INFO(Logger::m_logger, __VA_ARGS__)
#define PSDR_DEBUG(...) SPDLOG_LOGGER_DEBUG(Logger::m_logger, __VA_ARGS__)
#define PSDR_WARN(...)  SPDLOG_LOGGER_WARN(Logger::m_logger, __VA_ARGS__)
#define PSDR_ERROR(...) SPDLOG_LOGGER_ERROR(Logger::m_logger, __VA_ARGS__)
/// Throw an exception
#define Throw(...)                                          \
    do {                                                    \
        PSDR_ERROR(__VA_ARGS__);                            \
        throw std::runtime_error(fmt::format(__VA_ARGS__)); \
    } while (0)
// -----------------------------------------------------------------------------
//
// Assertions
//
// -----------------------------------------------------------------------------

#define PSDR_ASSERT(condition)                                                \
    if (!(condition)) {                                                       \
        std::stringstream ss;                                                 \
        ss << "Assertion failed: " << #condition << " in " << __FILE__ << ":" \
           << __LINE__;                                                       \
        throw std::runtime_error(ss.str());                                   \
    }

#define PSDR_ASSERT_MSG(condition, ...) \
    if (!(condition)) {                 \
        Throw(__VA_ARGS__);             \
    }
