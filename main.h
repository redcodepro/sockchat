#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <deque>
#include <mutex>
#include <regex>
#include <random>
#include <unordered_map>
#include <fstream>

#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#ifdef _WINDOWS
#	include <io.h>
#else
#	include <unistd.h>
#endif

#define ENET_STATIC
#include <enet.h>

#include "sc_types.h"

#define _printf __printf_stdout
#define _delete(p) if (p) { delete p; p = nullptr; }

typedef ENetPeer* peer_t;

namespace std {
	template <>
	struct hash<peer_t>
	{
		std::size_t operator()(const peer_t& k) const
		{
			return (std::size_t&)(k);
		}
	};
}

struct userdata_t
{
	int			m_id;
	std::string m_nick;
	int			m_color;
	std::string m_prefix;
	int         m_status;
};

#include "udp-packet.h"
#include "sqlite.h"
#include "udp-cryptor.h"
#include "udp-user.h"
#include "udp-server.h"
#include "cmd-handler.h"
#include "chat-handler.h"

extern database_t db;
extern udpserver_t server;
extern cmd_handler_t cmds;
extern chat_handler_t chat;
extern server_config_t cfg;

std::string format_out(std::string_view in, bool remove_color = true);
bool nick_is_valid(std::string_view nick, bool check_min = true);
int string_replace_all(std::string& str, const char* from, const char* to);
void init_commands();
void format_nick(std::string& out, user_t* user, user_t* target = nullptr);

#endif // !MAIN_H
