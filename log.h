#include "main.h"

class logger_t
{
private:
	FILE* m_fs = nullptr;
public:
	logger_t() {}
	~logger_t() { end(); }

	void begin(const char* filename);
	void end();

	void log(const char* fmt, ...);
};

extern logger_t _g_log;