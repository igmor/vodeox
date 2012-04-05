#include "config.h"

#include "Logger.h"
#include <sstream>

namespace vodeox
{

static const unsigned int MAX_SNPRINTF_BUF_SIZE = 2048;

//MAX_PATH has too many issues, we just need reasonable buffer size
static const unsigned int MAX_PATH_LENGTH = 2048;

//windows renamed posix string functions to an unsafe version _s*
#ifdef _WIN32
#define snprintf _snprintf
#endif

std::string DefaultLoggerFormatter::format( const Logger& logger, 
											const std::string& delim, 
											const LogEntry& logEntry)
{
	std::ostringstream s(std::ostringstream::out);
	s << logEntry.ts << delim 
		<< Logger::getLevelToken(logger.getLevel()) << delim 
		<< logEntry.component << delim
		<< logEntry.file << delim
		<< logEntry.line << delim
		<< logEntry.message << '\n';

	return s.str();
}

//Logger implementation
Logger::Logger() : 
	m_stream(NULL), 
	m_logLevel(LOG_LEVEL_WARN), 
	m_bRunning(false),
	m_delim(" ")
{
}

Logger::~Logger()
{
	stopQueue();
	if (m_stream!=NULL)
		fclose(m_stream);
}

std::string Logger::getLevelToken(Logger::LOGLEVEL level)
{
	switch (level)
	{
		case LOG_LEVEL_SILENT: return "SILENT"; break;
		case LOG_LEVEL_FATAL: return "FATAL"; break;
		case LOG_LEVEL_ERROR: return "ERROR"; break;
		case LOG_LEVEL_WARN: return "WARN"; break;
		case LOG_LEVEL_INFO: return "INFO"; break;
		case LOG_LEVEL_DEBUG: return "DEBUG"; break;
		case LOG_LEVEL_VERBOSE: return "VERBOSE"; break;
		default:
			return "UNDEFINED_LOG_LEVEL";
	}	
}

int Logger::log(LOGLEVEL level, 
				const char* file, 
				int line, 
				const vodeox::time& ts, 
				const char* component, 
				const char* fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	int ret = instance()._log(level, file, line, ts, component, fmt,arglist);
	va_end(arglist);
	return ret;
}

void Logger::_setFileName(const std::string& name)
{
	stopQueue();

	if (m_stream != NULL)
		fclose(m_stream);

	if ((m_stream = fopen(name.c_str(), "w+")) == NULL)
		fprintf(stderr, "couldn't open file for logging %s \n", name.c_str());

	m_archivefmt = name;
	m_filepath = name;
    size_t pos = name.rfind('/');
	const char* logfname = (pos == std::string::npos) ? name.c_str() : name.substr(pos).c_str();
    const char* logbase = strrchr(logfname,'.');

	if (!logbase)
		m_archivefmt += "-%s";
	else
	{
		int pos = m_archivefmt.find_last_of('.');
		m_archivefmt.replace(pos, 1, "-%s.");
	}

	startQueue();
}

void Logger::_setRotationSize(long sz)
{
	scoped_lock lock(m_fop_mutex);
	m_rotatepos = sz;
}

int Logger::_log(LOGLEVEL level,  
				 const char* file, 
				 int line, 
				 const vodeox::time& ts, 
				 const char* component, 
				 const char* fmt, 
				 va_list arglist)
{
	std::string buf;
	buf.resize(MAX_SNPRINTF_BUF_SIZE);

	if (level < m_logLevel) 
		return -1;
	else
	{
		int ret = vsnprintf(&buf[0], MAX_SNPRINTF_BUF_SIZE - 1, fmt, arglist);
		LogEntry l_entry(file, line, component, ts, buf);
		m_queue.push(l_entry);
		return ret;
	}
}

void Logger::startQueue()	
{
	m_bRunning = true;
    vodeox::thread::start(reinterpret_cast<void*>(this));
}

void Logger::stopQueue()
{
	m_bRunning = false;
	m_queue.shutdown();
	join();

	//clear the queue
	std::vector<LogEntry> logEntries;
	m_queue.pop(logEntries);

	format(logEntries);
}

void Logger::rotateFile()
{            
	if (!m_stream)
		return;

	if (m_rotatepos > 0)
	{
		fclose(m_stream);
		char newname[MAX_PATH_LENGTH+1];
		{
             struct tm t;
             time_t tt = ::time(NULL);
             localtime_r(&tt, &t);
             char dbuf[300];
             strftime(dbuf, sizeof(dbuf), "%Y-%m-%d-%H%M%S",&t);

			 snprintf(newname, sizeof(newname), m_archivefmt.c_str(), dbuf);
		}

		if (0 != rename(m_filepath.c_str(),newname))
			fprintf(stderr, "coundn't rotate files from %s to %s, error = %d", m_filepath.c_str(), newname, errno);
		m_stream = fopen(m_filepath.c_str(), "w+");
	}
}

void Logger::format(const std::vector<LogEntry>& entries)
{
	for (unsigned int i = 0; i < entries.size(); i++)
		if (m_formatter)
		{
			std::string message = m_formatter->format(*this, m_delim, entries[i]);
			if (m_stream)
			{
				fprintf(m_stream, "%s\n", message.c_str());
				fflush(m_stream);

				scoped_lock lock(m_fop_mutex);

				long  cur = ftell(m_stream);
				if (cur >= m_rotatepos)
					rotateFile();
			}
		}
}

void Logger::run()
{
	while (m_bRunning)
	{
		std::vector<LogEntry> logEntries;
		m_queue.wait_and_pop(logEntries);

		format (logEntries);
	}
}

} //namespace vodeox
