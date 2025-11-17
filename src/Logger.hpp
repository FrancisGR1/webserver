#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>

class Logger 
{
	public: 

	Logger() = delete;

	// https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels
	enum LogLevel
	{
		TRACE,  //very specific parts of procedures
		DEBUG,  //helpful diagnostics
		INFO,   //useful information
		WARN,   //can potentially cause weird behavior
		ERROR,  //error fatal to the operation 
		FATAL,  //error fatal to the application
		NONE
	};

	static void trace(const char* fmt, ...); 
	static void debug(const char* fmt, ...); 
	static void info(const char* fmt, ...);  
	static void warn(const char* fmt, ...);  
	static void error(const char* fmt, ...); 
	static void fatal(const char* fmt, ...); 

	template<typename logT> static void trace(const logT& to_log);
	template<typename logT> static void debug(const logT& to_log);

	static void setOutput(const char* file);
	static void closeOutput();

	static void setGlobalLevel(LogLevel level);


	private:

	static std::ostream* m_log_stream;
	static std::ofstream* m_file_stream;
	static std::vector<std::ofstream*> m_opened_files;

	static LogLevel m_global_level;

	static void vlog(LogLevel level, const char* fmt, va_list args);
	template<typename Tlog> void tlog(LogLevel level, const Tlog& to_log);
	static bool shouldLog(LogLevel level);
	static std::string logLeveltoString(LogLevel level);
	static std::string logLeveltoColor(LogLevel level);
};

#include "Logger_templates.cpp"
