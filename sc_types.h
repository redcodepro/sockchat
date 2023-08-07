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
	std::string name = "Sockchat LUA v0.1-alpha";
	int			port = 8000;
	int			slots = 100;
	std::string logfile = "";

	void load_cmdline(int argc, char** argv);
} server_config_t;

bool redirect_stdout(const char* filename);
