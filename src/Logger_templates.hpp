#ifndef LOGGER_TEMPLATES_HPP
# define LOGGER_TEMPLATES_HPP

//@TODO: mudar extensão para .tpp -> incluir .tpp na extensões de cpp

#include <iostream>
#include "Logger.hpp"

template<typename logT>
void Logger::trace(const logT& to_log)
{
	tlog(TRACE, to_log);
}

template<typename logT>
void Logger::debug(const logT& to_log)
{
	tlog(DEBUG, to_log);
}

template<typename Tlog>
void Logger::tlog(LogLevel level, const Tlog& to_log)
{
	if (m_log_stream == &std::cout)
		(*m_log_stream) << logLeveltoColor(level) << "[" << logLeveltoString(level) << "] " << "\033[0m";
	else
		(*m_log_stream) << "[" << logLeveltoString(level) << "] ";

	(*m_log_stream) << __FILE__ << " " << __LINE__ << " " << to_log << std::endl;
}
# endif //LOGGER_TEMPLATES_HPP
