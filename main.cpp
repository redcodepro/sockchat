#include "main.h" // ÙŦF-8

#define UDP_ADDRESS "0.0.0.0"
#define UDP_PORT	7777

#define DB_ADDRESS	"localhost"
#define DB_USER		"sockchat"
#define DB_PASSWORD	"uxQZnTp2"
#define DB_SCHEMA	"sockchat"

mysql_t			db;
udpserver_t		server;
cmd_handler_t	cmds;
chat_handler_t	chat;

int main(int argc, char* argv[])
{
	_printf("[info] Sockchat/UDP v1.1 started");

	if (!db.init(DB_ADDRESS, DB_USER, DB_PASSWORD, DB_SCHEMA))
		return 1;

	if (!server.bind(UDP_ADDRESS, UDP_PORT))
		return 2;

	init_commands();
	db.load_banip();

	server.exec();
	
	db.free();

	return 0;
}
