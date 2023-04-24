﻿#include "main.h" // ÙŦF-8

std::string create_key(std::size_t len)
{
	std::string result;
	srand(time(0) + rand());
	do
	{
		char c = (char)(rand() & 0xFF);
		if ((c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z'))
			result.push_back(c);
	} while (result.size() != len);
	return result;
}

color_t create_color()
{
	color_t result = 0xFF000000;
	srand(time(0) + rand());
	result |= (rand() & 0xFF) << 8 * 2;
	result |= (rand() & 0xFF) << 8 * 1;
	result |= (rand() & 0xFF) << 8 * 0;
	return result;
}

void trim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

std::string format_out(const std::string& in, bool remove_color)
{
	std::string result;

	// filter: size, bad chars
	for (auto &c : in)
	{
		if ((uint8_t)c < 32)
			continue;
		
		result += c;
		
		if (result.size() >= 128)
			break;
	}

	// filter: trim
	trim(result);

	// filter: color
	while (remove_color)
	{
		static std::regex tag_regex("\\{[A-Fa-f0-9]{6}\\}");
		std::string out = std::regex_replace(result, tag_regex, "");
		if (out.size() == result.size())
			break;
		result = out;
	}

	// filter: make smiles
	const char* smiles[][2] = {

		{":)", "\uf118"}, {">:(","\uf556"}, {":|", "\uf11a"},
		{";)", "\uf58c"}, {":(", "\uf119"}, {":D", "\uf599"}, {":'(","\uf5b4"},
		{":o", "\uf5c2"}, {":p", "\uf58a"}, {"=)", "\uf581"}, {"xD", "\uf586"},
		{"(r)","\uf25d"}, {"(c)","\uf1f9"}, {"(+)","\uf164"}, {"(-)","\uf165"},
		{"(*)","\uf005"}, {"($)","\uf3d1"}, {"(%)","\uf3a5"}, {"<3", "\uf004"},
		{"0_0","\uf579"}, {":*", "\uf598"},
	};

	for (auto it : smiles)
		string_replace_all(result, it[0], it[1]);

	return result;
}

void udpserver_t::send_online()
{
	time_t _time = time(0);
	// update period: 30s or onchange
	if ((_time - m_lou) < 30)
		return;
	m_lou = _time;

	char buf[256];
	int users = 0;
	int total = 0;
	std::string online;

	for (auto& it : m_users)
	{
		user_t* user = it.second;

		if (user->m_status == 0)
		{
			// nope
		}
		else
		{
			if (user->m_hideme)
				continue;
			
			sprintf(buf, "\n%s (%d) ", user->nick_c(), user->m_status);
			online += buf;

			if (user->m_status < 4)
				online += user->m_watching ? "\uf11a" : "\uf111";
			
			users += 1;
		}
		total += 1;
	}

	sprintf(buf, "\uf007 %d / %d | Онлайн:", users, total);
	online.insert(0, buf);

	opacket_t packet(id_hudtext_init);
	packet.write_string(online);
	Broadcast(&packet, 0);
}

bool key_is_valid(const std::string& key)
{
	if (key.size() == 0)
		return false;

	for (char c : key)
		if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
			return false;
	return true;
}

bool nick_is_valid(const std::string& nick, bool check_min)
{
	if ((nick.size() < 5 && check_min) || nick.size() > 24)
		return false;

	for (char c : nick)
		if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
			return false;
	return true;
}

std::vector<std::string> split(const std::string& in, char delimiter, bool allow_empty)
{
	std::vector<std::string> out;
	size_t prev_pos = 0, pos = 0;

	do
	{
		pos = in.find(delimiter, pos);

		std::string res = in.substr(prev_pos, pos - prev_pos);

		if (allow_empty || res.size())
			out.push_back(res);

		if (pos != std::string::npos)
			prev_pos = ++pos;

	} while (pos != std::string::npos);

	return out;
}

int string_replace_all(std::string& str, const char* from, const char* to)
{
	int count = 0;
	size_t start_pos = 0;
	size_t len_from = strlen(from);
	size_t len_to = strlen(to);
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, len_from, to);
		start_pos += len_to;
		count++;
	}
	return count;
};

std::string md5(const std::string& in)
{
	MD5 hash;
	hash.add(in.data(), in.size());
	return hash.getHash();
}

const char* addr(ENetAddress* addr)
{
	char ip[256];
	enet_address_get_host_ip(addr, ip, 256);
	
	static char buf[256];
	snprintf(buf, 256, "%s:%hu", ip, addr->port);
	return buf;
}

std::string addr_ip(ENetAddress* addr)
{
	char ip[256];
	enet_address_get_host_ip(addr, ip, 256);
	return ip;
}

std::string urlencode(const std::string& in)
{
	std::string out;

	for (const char& c : in)
	{
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			c == '-' || c == '_' || c == '.' ||
			c == '!' || c == '~' || c == '*' ||
			c == '\'' || c == '(' || c == ')')
		{
			out += c;
		}
		else
		{
			char buf[32];
			sprintf(buf, "%%%02hhx", c);
			out += buf;
		}
	}
	return out;
}

std::string find_audio_url(const std::string& name)
{
	std::size_t p = name.find_last_of('/');
	std::string f = (p == std::string::npos ? name : name.substr(p + 1)) + ".mp3";
	std::string n = "/var/www/html/audio/" + f;
	struct stat s;
	if (stat(n.c_str(), &s) != 0)
		return "";
	if (!(s.st_mode & S_IFREG))
		return "";
	return std::string("http://xcht.tk/audio/" + f);
}
