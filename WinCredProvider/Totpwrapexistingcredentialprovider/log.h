
#ifndef _LOG_H_
#define _LOG_H_

/* logger module to be used for all kinds of logging */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define ENABLE_LOG 1  //RELEASE

/*UNIX max path for this log file */
#define MAXIMUMPATH 128

#define ERR_LOG_DISABLED -1

namespace Logging {

	/* How much logging do we need for any given file */
	enum eLogType {
		NONE   = 0,
		INFO   = 1, // also print state,etc
		ERR = 2,	// print only errors
		WARN   = 3,	// give warnings like setPixel error
		DEBUG  = 4, // all kinds of debugging output
		DEBUG2 = 5, // all kinds of debugging output
		DEBUG3 = 6, // all kinds of debugging output
		ALL    = 7 };// everything

	class Logger
	{

		public:

			/* default log file to which everything is logged */
			Logger();

			/* specify your own logfile and log to it */
			Logger(char* cLogfile);

			/* default destructor */
			~Logger();

			/* set log level for this . default is 0 */
			int SetLogLevel(eLogType eLogLevel);

			/* string value to be logged */
			int LogS(eLogType logLevel, const char* format,...);
	
			int LogS(const char* format,...);
			// logLevel = DEBUG

			int setLogTime (bool loggingOn);

		private:
			/* log file name. This will remain open as long as
			   we dont explicitly destroy this object. If it
			   already exists then we move the previous one
			   to .log_ and restart logging on this one
			 */

			char m_sLogFile[MAXIMUMPATH];

			/* set log level */
			eLogType m_eLogLevel;

			/* file pointer to the log file. using fprintf for now */
			FILE* m_pfLogFile;

			/* ref count for this file ,if we already have it open */
			int iRefCount;

			bool m_logTimeAlso;
	};
};
#undef MAXIMUMPATH
#endif //_LOG_H_
