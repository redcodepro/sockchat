﻿#include "main.h" // ÙŦF-8

database_t		db;
udpserver_t		server;
cmd_handler_t	cmds;
chat_handler_t	chat;
server_config_t cfg;

int main(int argc, char* argv[])
{
	_printf("[info] Sockchat/UDP v1.2 started");

	cfg.load_cmdline(argc, argv);

	if (!cfg.logfile.empty() && !redirect_stdout(cfg.logfile.c_str()))
		return 1;

	if (cfg.port < 0 || cfg.port > 65535)
	{
		printf("[error] incorrect port value (0 - 65525)\n");
		return 2;
	}

	if (cfg.slots < 2 || cfg.slots > 4096)
	{
		printf("[error] incorrect slots value (2 - 4096)\n");
		return 2;
	}

	if (!server.init(&cfg))
	{
		_printf("[error] Failed to init server.");
		return 3;
	}

#if (MYSQL_ENABLED == 1)
	if (!db.init(MYSQL_ADDRESS, MYSQL_USER, MYSQL_PASSWORD, MYSQL_SCHEMA))
#else
	if (!db.init(SQLITE_FILENAME))
#endif
	{
		_printf("[error] Failed to init database.");
		return 4;
	}

	init_commands();

	server.exec();
	
	db.free();

	return 0;
}
