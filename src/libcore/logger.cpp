#include <core/logger.h>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

ref<spdlog::logger>        Logger::m_logger;
std::vector<Logger::Entry> Logger::m_logs;
spdlog::sink_ptr           Logger::m_stdout_sink;
spdlog::sink_ptr           Logger::m_file_sink;

void Logger::static_init() {
    std::vector<spdlog::sink_ptr> sinks;
    m_stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    m_file_sink   = std::make_shared<spdlog::sinks::basic_file_sink_mt>("PSDR.log", true);
    sinks.emplace_back(m_stdout_sink);
    sinks.emplace_back(m_file_sink);

    m_logger = std::make_shared<spdlog::logger>("PSDR", begin(sinks), end(sinks));

    spdlog::register_logger(m_logger);
    m_logger->set_level(spdlog::level::trace);
    m_logger->flush_on(spdlog::level::trace);
}

void Logger::set_level(spdlog::level::level_enum level) {
    m_stdout_sink->set_level(level);
}

void Logger::info(Logger::Entry entry) {
    m_logs.push_back(entry);
    m_logger->info(entry.msg);
}

void Logger::debug(Logger::Entry entry) {
    m_logs.push_back(entry);
    m_logger->debug(entry.msg);
}

void Logger::warn(Logger::Entry entry) {
    m_logs.push_back(entry);
    m_logger->warn(entry.msg);
}

void Logger::error(Logger::Entry entry) {
    m_logs.push_back(entry);
    m_logger->error(entry.msg);
}