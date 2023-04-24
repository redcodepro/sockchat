#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <deque>
#include <mutex>
#include <regex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#define ENET_STATIC
#include "enet/enet.h"

#include "config.h"

#include "md5.h"
#include "log.h"

typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET_CLOSE(s) if (s) { close(s); s = INVALID_SOCKET; }

#define _printf _g_log.log
#define _delete(p) if (p) { delete p; p = nullptr; }

typedef ENetPeer* peer_t;

namespace std {
	template <>
	struct hash<peer_t>
	{
		std::size_t operator()(const peer_t& k) const
		{
			return *(std::size_t*)(&k);
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
#include "mysql.h"
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

std::string create_key(std::size_t len);
color_t create_color();

std::string format_out(const std::string& in, bool remove_color = true);
bool key_is_valid(const std::string& key);
bool nick_is_valid(const std::string& nick, bool check_min = true);
std::vector<std::string> split(const std::string& in, char delimiter, bool allow_empty = false);
int string_replace_all(std::string& str, const char* from, const char* to);
std::string md5(const std::string& in);
const char* addr(ENetAddress* addr);
std::string addr_ip(ENetAddress* addr);
void init_commands();
std::string urlencode(const std::string& in);
std::string find_audio_url(const std::string& name);

#endif // !MAIN_H
