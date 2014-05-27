
#include "log.h"
#include <time.h>
#include <sys/utime.h>
/* Default Constructor for the logfile. This file is named default.log
   Default log level is 0, which means no Logging.
*/

namespace Logging {

	Logger::Logger()
	{
		m_pfLogFile = NULL;
#ifdef DEBUG
		m_eLogLevel = DEBUG;
#else
		m_eLogLevel = NONE;
#endif
		iRefCount=0;
		if(ENABLE_LOG)
		{
			strncpy(m_sLogFile,"default.log",12);
			m_pfLogFile = fopen(m_sLogFile,"a+");
			if (m_pfLogFile)
				iRefCount++;
		}
		m_logTimeAlso = true;
	}

	Logger::Logger(char* cLogFile)
	{
		m_pfLogFile = NULL;
#ifdef DEBUG
		m_eLogLevel = DEBUG;
#else
		m_eLogLevel = INFO;
#endif
		iRefCount = 0;
		if(ENABLE_LOG)
		{
			strncpy(m_sLogFile,cLogFile,80);
			m_pfLogFile = fopen(m_sLogFile,"a+");
			if (m_pfLogFile)
				iRefCount++;
		}
		m_logTimeAlso = true;
	}

	Logger::~Logger()
	{
		if (m_pfLogFile)
		{
			fclose(m_pfLogFile);
			iRefCount--;
		}
	}

	int Logger::SetLogLevel(eLogType eLogLevel)
	{
		
		if(!ENABLE_LOG)  return ERR_LOG_DISABLED;

		m_eLogLevel = eLogLevel;

		return eLogLevel;
	}

	int Logger::setLogTime(bool loggingOn)
	{

		if(!ENABLE_LOG)  return ERR_LOG_DISABLED;

		m_logTimeAlso = loggingOn;

		return 0;
	}

	int Logger::LogS(eLogType logLevel, const char* format,...)
	{

		if(!ENABLE_LOG)  return ERR_LOG_DISABLED;

		if(m_eLogLevel < logLevel) return logLevel;

		int iReturn = 0;

		/* needed to record the time of the log */
		
		//timezone tz;

		/* have a variable argument list */
		va_list vAList;

		/* initialise the argument list */
		va_start(vAList,format);

		/* call vfprintf to print to the file stream */
		if ((iRefCount)&&(m_pfLogFile))
		{
			time_t cur_timep = time(NULL);
			struct tm* cur_time = localtime(&cur_timep);

			if(m_logTimeAlso)
			{
				fprintf(m_pfLogFile,"<%02d/%02d/%04d %02d:%02d:%02d %06ld %d> ",
						cur_time->tm_mday, cur_time->tm_mon+1, cur_time->tm_year+1900,
						cur_time->tm_hour, cur_time->tm_min,   cur_time->tm_sec, 0,logLevel);
			}
			else
			{
				fprintf(m_pfLogFile,"<%d> ",logLevel);
			}

			iReturn = vfprintf(m_pfLogFile,format,vAList);
			fflush(m_pfLogFile);
			va_end(vAList);
		}
		return iReturn;
	}

	int Logger::LogS(const char* format,...)
	{

		if(!ENABLE_LOG)  return ERR_LOG_DISABLED;

		eLogType logLevel = INFO;

		if(m_eLogLevel < logLevel) return logLevel;

		int iReturn = 0;

		/* needed to record the time of the log */
		//timezone tz;

		/* have a variable argument list */
		va_list vAList;

		/* initialise the argument list */
		va_start(vAList,format);

		/* call vfprintf to print to the file stream */
		if ((iRefCount)&&(m_pfLogFile))
		{
			if(m_logTimeAlso)
			{
				time_t cur_timep = time(NULL);
				struct tm* cur_time = localtime(&cur_timep);

				fprintf(m_pfLogFile,"<%02d/%02d/%04d %02d:%02d:%02d %06ld %d> ",
						cur_time->tm_mday, cur_time->tm_mon+1, cur_time->tm_year+1900, 
						cur_time->tm_hour, cur_time->tm_min,cur_time->tm_sec,0,logLevel);
				//fprintf(m_pfLogFile,"<%ld %06ld %d> ",tv.tv_sec,tv.tv_usec,logLevel);
			}
			else
			{
				fprintf(m_pfLogFile,"<%d> ",logLevel);
			}

			iReturn = vfprintf(m_pfLogFile,format,vAList);
			fflush(m_pfLogFile);
			va_end(vAList);
		}
		return iReturn;
	}
};

