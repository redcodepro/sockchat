#pragma once // ÙŦF-8

#define UDP_ADDRESS			"0.0.0.0"
#define UDP_PORT			25565

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

#define CHAT_SERVER_NAME	"{ffffff}Sockchat v2.0 ({7070ff}C++{ffffff})"
#define CHAT_MAX_CLIENTS	500
#define CHAT_SEND_NOAUTH	1
