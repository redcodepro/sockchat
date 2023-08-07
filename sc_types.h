#pragma once

#include <string>

typedef unsigned int	id_t;
typedef unsigned short	len_t;
typedef unsigned int	color_t;

static_assert(sizeof(id_t) == 4);
static_assert(sizeof(len_t) == 2);
static_assert(sizeof(color_t) == 4);

void _srand();

color_t create_color();

std::string md5(std::string in);
std::string create_key(std::size_t length);
std::string urlencode(std::string in);

typedef struct _server_config_t
{
	std::string name = "{ffffff}Sockchat v2.0 ({7070ff}C++{ffffff})";
	int			port = 8000;
	int			slots = 500;
	std::string logfile = "";

	std::string mysql_address	= "localhost";
	std::string mysql_user		= "sockchat";
	std::string mysql_pass		= "12345678";
	std::string mysql_schema	= "sockchat";

	std::string sqlite_filename	= "sockchat.db";

	void load_cmdline(int argc, char** argv);
} server_config_t;

bool redirect_stdout(const char* filename);
