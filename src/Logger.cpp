#include <fstream>
#include <ostream>
#include <iostream>
#include <cstdarg>
#include "Logger.hpp"
#include "constants.hpp"

std::ostream* Logger::m_log_stream = &std::cout;
std::ofstream* Logger::m_file_stream;
std::vector<std::ofstream*> Logger::m_opened_files;

Log::Level Logger::m_global_level = Log::Trace;

void Logger::trace(const char* fmt, ...)
{	
	if (should_log(Log::Trace))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Trace, fmt, args);
		va_end(args);
	}
};

void Logger::debug(const char* fmt, ...)
{	
	if (should_log(Log::Debug))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Debug, fmt, args);
		va_end(args);
	}
};

void Logger::info(const char* fmt, ...)
{	
	if (should_log(Log::Info))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Info, fmt, args);
		va_end(args);
	}
};

void Logger::warn(const char* fmt, ...)
{	
	if (should_log(Log::Warn))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Warn, fmt, args);
		va_end(args);
	}
};

void Logger::error(const char* fmt, ...)
{	
	if (should_log(Log::Error))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Error, fmt, args);
		va_end(args);
	}
};

void Logger::fatal(const char* fmt, ...)
{	
	if (should_log(Log::Fatal))
	{
		va_list args;
		va_start(args, fmt);
		vlog(Log::Fatal, fmt, args);
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
	for (size_t i = 0; i < m_opened_files.size(); i++)
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
		(*m_log_stream) << log_level_to_color(level) << "[" << log_level_to_string(level) << "] " << constants::RESET;
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
		case Log::Trace: return "Trace";
		case Log::Debug: return "Debug";
		case Log::Info:  return "Info";
		case Log::Warn:  return "Warn";
		case Log::Error: return "Error";
		case Log::Fatal: return "Fatal";
		default: return "None";
	}
	return ("None");
}

std::string Logger::log_level_to_color(Log::Level level)
{
	switch (level)
	{
		case Log::Trace: return constants::CYAN;
		case Log::Debug: return constants::BLUE;
		case Log::Info:  return constants::GREEN;
		case Log::Warn:  return constants::YELLOW;
		case Log::Error: return constants::RED;
		case Log::Fatal: return constants::BRIGHT_RED;
		default:    		return constants::RESET;
	}
	return constants::RESET;
}
