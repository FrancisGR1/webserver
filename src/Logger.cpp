#include <fstream>
#include <ostream>
#include <iostream>
#include <cstdarg>
#include "Logger.hpp"

std::ostream* Logger::m_log_stream = &std::cout;
std::ofstream* Logger::m_file_stream;
std::vector<std::ofstream*> Logger::m_opened_files;

Log::Level Logger::m_global_level = Log::Level::Trace;

void Logger::trace(const char* fmt, ...)
{	
	if (should_log(Log::Level::Trace))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Trace, fmt, args);
		va_end(args);
	}
};

void Logger::debug(const char* fmt, ...)
{	
	if (should_log(Log::Level::Debug))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Debug, fmt, args);
		va_end(args);
	}
};

void Logger::info(const char* fmt, ...)
{	
	if (should_log(Log::Level::Info))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Info, fmt, args);
		va_end(args);
	}
};

void Logger::warn(const char* fmt, ...)
{	
	if (should_log(Log::Level::Warn))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Warn, fmt, args);
		va_end(args);
	}
};

void Logger::error(const char* fmt, ...)
{	
	if (should_log(Log::Level::Error))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Error, fmt, args);
		va_end(args);
	}
};

void Logger::fatal(const char* fmt, ...)
{	
	if (should_log(Log::Level::Fatal))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Level::Fatal, fmt, args);
		va_end(args);
	}
};

void Logger::set_output(const char* file)
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

void Logger::close_output()
{
	for (int i = 0; i < m_opened_files.size(); i++)
	{
		std::ofstream* file = m_opened_files.at(i);
		file->close();
	}
}

void Logger::set_global_level(Log::Level level)
{
	m_global_level = level;
}

void Logger::vlog(Log::Level level, const char* fmt, va_list args)
{
	char buffer[5000];
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);

	if (m_log_stream == &std::cout)
		(*m_log_stream) << log_level_to_color(level) << "[" << log_level_to_string(level) << "] " << "\033[0m";
	else
		(*m_log_stream) << "[" << log_level_to_string(level) << "] ";

	(*m_log_stream) << buffer << "\n";
}


bool Logger::should_log(Log::Level level)
{
	if (level >= m_global_level)
		return true;
	return (false);
};

std::string Logger::log_level_to_string(Log::Level level)
{
	switch (level)
	{
		case Log::Level::Trace: return "Trace";
		case Log::Level::Debug: return "Debug";
		case Log::Level::Info:  return "Info";
		case Log::Level::Warn:  return "Warn";
		case Log::Level::Error: return "Error";
		case Log::Level::Fatal: return "Fatal";
		default: return "None";
	}
	return ("None");
}

std::string Logger::log_level_to_color(Log::Level level)
{
	switch (level)
	{
		case Log::Level::Trace: return "\033[36m";  // Cyan
		case Log::Level::Debug: return "\033[34m";  // Blue
		case Log::Level::Info:  return "\033[32m";  // Green
		case Log::Level::Warn:  return "\033[33m";  // Yellow
		case Log::Level::Error: return "\033[31m";  // Red
		case Log::Level::Fatal: return "\033[91m";  // Bright red
		default:    		return "\033[0m";   // Reset
	}
	return "\033[0m";
}
