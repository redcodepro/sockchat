#include "main.h"

void logger_t::begin(const char* filename)
{
	if (m_fs == nullptr && filename != nullptr)
		m_fs = fopen(filename, "a");
	if (m_fs == nullptr)
		m_fs = stdout;

	fprintf(m_fs, "\n\n");
	fflush(m_fs);
}

void logger_t::end()
{
	if (m_fs == stdout)
		return;

	if (m_fs != nullptr)
	{
		fclose(m_fs);
		m_fs = nullptr;
	}
}

void logger_t::log(const char* fmt, ...)
{
	time_t t_ = time(0);
	struct tm tm_;
	localtime_r(&t_, &tm_);

	fprintf(m_fs, "[%02d:%02d:%02d] ", tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

	va_list va;
	va_start(va, fmt);
	vfprintf(m_fs, fmt, va);
	va_end(va);

	fprintf(m_fs, "\n");
	fflush(m_fs);
}

logger_t _g_log;