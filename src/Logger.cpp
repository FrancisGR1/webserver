#include <fstream>
#include <ostream>
#include <iostream>
#include <cstdarg>
#include "Logger.hpp"

std::ostream* Logger::m_log_stream = &std::cout;
std::ofstream* Logger::m_file_stream;
std::vector<std::ofstream*> Logger::m_opened_files;

Logger::LogLevel Logger::m_global_level = Logger::FATAL;

void Logger::trace(const char* fmt, ...)
{	
	if (shouldLog(TRACE))
	{
		va_list args;
		va_start(args, fmt);
		vlog(TRACE, fmt, args);
		va_end(args);
	}
};

void Logger::debug(const char* fmt, ...)
{	
	if (shouldLog(DEBUG))
	{
		va_list args;
		va_start(args, fmt);
		vlog(DEBUG, fmt, args);
		va_end(args);
	}
};

void Logger::info(const char* fmt, ...)
{	
	if (shouldLog(INFO))
	{
		va_list args;
		va_start(args, fmt);
		vlog(INFO, fmt, args);
		va_end(args);
	}
};

void Logger::warn(const char* fmt, ...)
{	
	if (shouldLog(WARN))
	{
		va_list args;
		va_start(args, fmt);
		vlog(WARN, fmt, args);
		va_end(args);
	}
};

void Logger::error(const char* fmt, ...)
{	
	if (shouldLog(ERROR))
	{
		va_list args;
		va_start(args, fmt);
		vlog(ERROR, fmt, args);
		va_end(args);
	}
};

void Logger::fatal(const char* fmt, ...)
{	
	if (shouldLog(FATAL))
	{
		va_list args;
		va_start(args, fmt);
		vlog(FATAL, fmt, args);
		va_end(args);
	}
};

void Logger::setOutput(const char* file)
{
	m_file_stream->open(file, std::ios::out | std::ios::app);
	if (!m_file_stream->is_open())
	{
		m_log_stream = &std::cout;
		Logger::error("Couldn't open: %s", file);
		return;
	}

	m_opened_files.push_back(m_file_stream);

	m_log_stream = m_file_stream;
}

void Logger::closeOutput()
{
	for (int i = 0; i < m_opened_files.size(); i++)
	{
		std::ofstream* file = m_opened_files.at(i);
		file->close();
	}
}

void Logger::setGlobalLevel(LogLevel level)
{
	m_global_level = level;
}

void Logger::vlog(LogLevel level, const char* fmt, va_list args)
{
	char buffer[5000];
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);

	if (m_log_stream == &std::cout)
		(*m_log_stream) << logLeveltoColor(level) << "[" << logLeveltoString(level) << "] " << "\033[0m";
	else
		(*m_log_stream) << "[" << logLeveltoString(level) << "] ";

	(*m_log_stream) << buffer << "\n";
}


bool Logger::shouldLog(LogLevel level)
{
	if (level <= m_global_level)
		return true;
	return (false);
};

std::string Logger::logLeveltoString(LogLevel level)
{
	switch (level)
	{
		case TRACE: return "Trace";
		case DEBUG: return "Debug";
		case INFO:  return "Info";
		case WARN:  return "Warn";
		case ERROR: return "Error";
		case FATAL: return "Fatal";
		default: return "None";
	}
	return ("None");
}

std::string Logger::logLeveltoColor(LogLevel level)
{
	switch (level)
	{
		case TRACE: return "\033[36m";  // Cyan
		case DEBUG: return "\033[34m";  // Blue
		case INFO:  return "\033[32m";  // Green
		case WARN:  return "\033[33m";  // Yellow
		case ERROR: return "\033[31m";  // Red
		case FATAL: return "\033[91m";  // Bright red
		default:    return "\033[0m";   // Reset
	}
	return "\033[0m";
}
