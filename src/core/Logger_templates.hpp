#ifndef LOGGER_TEMPLATES_HPP
#define LOGGER_TEMPLATES_HPP

//@TODO: mudar extensão para .tpp -> incluir .tpp na extensões de cpp

#include <iostream>

#include "core/Logger.hpp"
#include "core/constants.hpp"

template <typename logT>
void Logger::trace_obj(const logT& to_log, const std::string prefix)
{
    if (should_log(Log::Trace))
    {
        tlog(Log::Trace, to_log, prefix);
    }
}

template <typename logT>
void Logger::debug_obj(const logT& to_log, const std::string prefix)
{
    if (should_log(Log::Debug))
    {
        tlog(Log::Debug, to_log, prefix);
    }
}

template <typename logT>
void Logger::info_obj(const logT& to_log, const std::string prefix)
{
    if (should_log(Log::Info))
    {
        tlog(Log::Info, to_log, prefix);
    }
}

// forward declaration
namespace constants
{
extern const char* reset;
}

template <typename Tlog>
void Logger::tlog(Log::Level level, const Tlog& to_log, const std::string prefix)
{
    if (m_log_stream == &std::cout)
        (*m_log_stream) << log_level_to_color(level) << "[" << log_level_to_string(level) << "] " << constants::reset;
    else
        (*m_log_stream) << "[" << log_level_to_string(level) << "] ";
    (*m_log_stream) << prefix << to_log << "\n";
}
#endif // LOGGER_TEMPLATES_HPP
