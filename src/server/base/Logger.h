#ifndef __BASE_LOGGER_H
#define __BASE_LOGGER_H

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>

#include <string>
#include <queue>

#include <tr1/memory>
#include <pthread.h>

#include "base/scoped_lock.h"
#include "base/time.h"
#include "base/concurrent_queue.h"

namespace vodeox
{

class Logger;

/*
 * simple container class for log entries 
 */
struct LogEntry
{
	std::string				 file;
	int						 line;
	std::string				 component;
    vodeox::time              ts;
	std::string				 message;

	LogEntry() {}

	LogEntry(const std::string& f, 
			 int l, 
			 const std::string& c, 
			 const vodeox::time& t, 
			 const std::string&  m) :
	file(f), line(l), component(c), ts(t), message(m) {}
};


template<class T>
class Singleton
{
 public:
    static T& instance()
    {
        static T s_instance;
        return s_instance;
    }
    virtual ~Singleton(){}

 protected:
    Singleton()  { }
 
};

/*
 * An interface for formatting output in a logger
 */
class LoggerFormatter
{
	public:
		virtual std::string  format(const Logger& logger, 
									 const std::string& delim, 
									 const LogEntry& logMessage) = 0;
};

/*
 * default implementation of a formatter. Add any fields you feel like worth outputing 
 * in a constructor and change format function accordingly.
 */
class DefaultLoggerFormatter : public LoggerFormatter
{
	public:
		virtual std::string  format(const Logger& logger, 
									 const std::string& delim, 
									 const LogEntry& logMessage);

};


/*
 *  Logger implementation sigleton. You should use something like this to start using logger
 *  Please note you may lose some messages if you try to log anything after the shutdown has already initiated
 * 	Logger::setOutputFormatter(boost::shared_ptr<LoggerFormatter>(new DefaultLoggerFormatter()));
 *	Logger::info("%s\n", "this is just a log message");
 *
 */

class Logger :  Singleton<Logger>, public vodeox::thread
{
	public:
		typedef enum {
			LOG_LEVEL_SILENT = -1,
			LOG_LEVEL_VERBOSE = 0,
			LOG_LEVEL_DEBUG,
			LOG_LEVEL_INFO,
			LOG_LEVEL_WARN,
			LOG_LEVEL_ERROR,
			LOG_LEVEL_FATAL
		} LOGLEVEL;

        Logger();
		virtual ~Logger();

	    /**
	     * Sets the logging level. All messages with lower priority will be ignored.
	     */
	    static void setLevel(LOGLEVEL level) { instance().m_logLevel = level; }

	    /**
	     * Gets the defaul logging level.
	     */
	    static LOGLEVEL getLevel() { return instance().m_logLevel; }

	    /**
	     * Sets the log file name
	     */
		static void setFileName(const std::string& name) { instance()._setFileName(name); }

	    /**
	     * Sets the log file name
	     */
		static void setRotationSize(long sz) { instance()._setRotationSize(sz); }

	    /**
	     * Log a message to a log file. The method returns immediately.
		 * A timestamp of the log message is taken at a time when log method is being called
		 * An actual logging will happen in a separate thread.
	     */
	    static int log(LOGLEVEL level, 
						const char* file, 
						int line, 
                        const vodeox::time& ts, 
						const char* component,
						const char* fmt, ...);
	
		/*
		 * Set's field delimiter, it's a blank space by default
		 */
		static void setOutputFormatter(std::tr1::shared_ptr<LoggerFormatter>& formatter)
		{
			instance().m_formatter = formatter;
		}

		static void stop()
		{
			instance().stopQueue();
		}

		static std::string getLevelToken(Logger::LOGLEVEL level);
		/*
		 * Set's field delimiter, it's a blank space by default
		 */

	#define LOG(NAME,LEVEL) \
        static int NAME(const char* file, int line, const vodeox::time& ts, const char* component, const char* fmt, ...) \
		{ \
			va_list ap; \
			va_start(ap, fmt); \
			int ret = instance()._log(LEVEL, file, line, ts, component, fmt, ap); \
			va_end(ap); \
			return ret; \
		}
	
	    LOG(fatal, LOG_LEVEL_FATAL)
	    LOG(error, LOG_LEVEL_ERROR)
	    LOG(warning, LOG_LEVEL_WARN)
	    LOG(info, LOG_LEVEL_INFO)
	    LOG(debug, LOG_LEVEL_DEBUG)
	    LOG(verbose, LOG_LEVEL_VERBOSE)
	    LOG(silent, LOG_LEVEL_SILENT)

	private:
		
		void _setFileName(const std::string& name);
		void _setRotationSize(long sz);

	    int _log(LOGLEVEL level, 
				 const char* file, 
				 int line, 
				 const vodeox::time& ts, 
				 const char* component,
				 const char* fmt, 
				 va_list arglist);

		void addToQueue(const std::string& message);

		void startQueue();
		void stopQueue();
		void run();
		void format(const std::vector<LogEntry>& entries);

		void rotateFile();
	private:

	    FILE*								m_stream;
	    LOGLEVEL							m_logLevel;
		std::string							m_delim;

        std::tr1::shared_ptr<LoggerFormatter>	m_formatter;

		volatile bool						m_bRunning;
		concurrent_queue<LogEntry>			m_queue;

		std::string							m_archivefmt;
		std::string							m_filepath;
		long								m_rotatepos;

		mutable mutex				        m_fop_mutex;
};

#define LOG_FATAL(component, ...)	Logger::fatal	(__FILE__,__LINE__, vodeox::time::now(), component, __VA_ARGS__)
#define LOG_ERROR(component,...)	Logger::error	(__FILE__,__LINE__, vodeox::time::now(), component, __VA_ARGS__)
#define LOG_WARN(component,...)		Logger::warning	(__FILE__,__LINE__, vodeox::time::now(), component,__VA_ARGS__)
#define LOG_INFO(component,...)		Logger::info	(__FILE__,__LINE__, vodeox::time::now(), component,__VA_ARGS__)
#define LOG_DEBUG(component,...)	Logger::debug	(__FILE__,__LINE__, vodeox::time::now(), component, __VA_ARGS__)
#define LOG_VERBOSE(component,...)	Logger::verbose	(__FILE__,__LINE__, vodeox::time::now(), component, __VA_ARGS__)
#define LOG_SILENT(component,...)	Logger::silent	(__FILE__,__LINE__, vodeox::time::now(), component, __VA_ARGS__)

} //namespace vodeox

#endif
