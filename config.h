#pragma once // ÙŦF-8

#define MYSQL_ENABLED		0

#if (MYSQL_ENABLED == 1)
#  define MYSQL_ADDRESS		"localhost"
#  define MYSQL_USER		"sockchat"
#  define MYSQL_PASSWORD	"uxQZnTp2"
#  define MYSQL_SCHEMA		"sockchat"
#else
#  define SQLITE_FILENAME	"sockchat.db"
#endif

#define CHAT_SEND_NOAUTH	1
