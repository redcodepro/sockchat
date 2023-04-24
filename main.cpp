#include "main.h" // ÙŦF-8

database_t		db;
udpserver_t		server;
cmd_handler_t	cmds;
chat_handler_t	chat;

int main(int argc, char* argv[])
{
	if (argc > 2 && !strcmp(argv[1], "-l"))
		_g_log.begin(argv[2]);
	else
		_g_log.begin(nullptr);

	_printf("[info] Sockchat/UDP v1.2 started");

#if (MYSQL_ENABLED == 1)
	if (!db.init(MYSQL_ADDRESS, MYSQL_USER, MYSQL_PASSWORD, MYSQL_SCHEMA))
#else
	if (!db.init(SQLITE_FILENAME))
#endif
	{
		_printf("[error] Failed to init database.");
		return 1;
	}

	if (!server.bind(UDP_ADDRESS, UDP_PORT))
		return 2;

	init_commands();

	server.exec();
	
	db.free();

	return 0;
}
