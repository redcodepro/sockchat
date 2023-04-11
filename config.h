#pragma once

#define UDP_ADDRESS			"0.0.0.0"
#define UDP_PORT			7777

#define MYSQL_ENABLED		0

#if (MYSQL_ENABLED == 1)
#  define MYSQL_ADDRESS		"localhost"
#  define MYSQL_USER		"sockchat"
#  define MYSQL_PASSWORD	"uxQZnTp2"
#  define MYSQL_SCHEMA		"sockchat"
#else
#  define SQLITE_FILENAME	"sockchat.db"
#endif

#define LOG_FILE			0
#define LOG_FILENAME		"sockchat.log"
