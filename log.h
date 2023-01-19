#include "main.h"

#define _LOG_TO_FILE	1
#define _LOG_FILENAME	"/home/sockchat.log"

class logger_t
{
private:
	FILE* m_fs;
	void begin(const char* filename);
	void end();
public:
	logger_t()
		: m_fs(nullptr)
	{
		begin(_LOG_FILENAME);
	}
	~logger_t()
	{
		end();
	}

	void log(const char* fmt, ...);
};

extern logger_t _g_log;