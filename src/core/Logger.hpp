#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <string>
#include <vector>

struct Log
{
	// https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels
	enum Level
	{
		Trace = 0,  // very specific parts of procedures
		Debug,  // helpful diagnostics
		Info,   // useful information
		Warn,   // can potentially cause weird behavior
		Error,  // error fatal to the operation 
		Fatal,  // error fatal to the application
		None
	};
};

class Logger 
{
	public: 

	static void trace(const char* fmt, ...); 
	static void debug(const char* fmt, ...); 
	static void info(const char* fmt, ...);  
	static void warn(const char* fmt, ...);  
	static void error(const char* fmt, ...); 
	static void fatal(const char* fmt, ...); 

	template<typename logT> static void trace_obj(const logT& to_log, const std::string prefix="");
	template<typename logT> static void debug_obj(const logT& to_log, const std::string prefix="");

	static void set_output(const char* file);
	static void close_output();

	static void set_global_level(Log::Level level);


	private:
	static std::ostream* m_log_stream;
	static std::ofstream* m_file_stream;
	static std::vector<std::ofstream*> m_opened_files;

	static Log::Level m_global_level;

	static void vlog(Log::Level level, const char* fmt, va_list args);
	template<typename Tlog> static void tlog(Log::Level level, const Tlog& to_log, const std::string prefix);
	static bool should_log(Log::Level level);
	static std::string log_level_to_string(Log::Level level);
	static std::string log_level_to_color(Log::Level level);
};

#include "Logger_templates.hpp"

# endif // LOGGER_HPP
