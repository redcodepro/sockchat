#include "main.h" // ÙŦF-8
#include "md5.h"

void _srand()
{
	srand((unsigned int)time(NULL) + (unsigned int)rand());
}

color_t create_color()
{
	color_t result = 0xFF000000;
	_srand();
	result |= (rand() & 0xFF) << 8 * 2;
	result |= (rand() & 0xFF) << 8 * 1;
	result |= (rand() & 0xFF) << 8 * 0;
	return result;
}

std::string md5(std::string in)
{
	return MD5()(in);
}

std::string create_key(std::size_t length)
{
	std::string result;
	_srand();
	do {
		char c = (char)(rand() & 0xFF);
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			result.push_back(c);
	} while (result.size() != length);
	return result;
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

std::string addr_ip(ENetAddress* addr)
{
	char ip[256];
	enet_address_get_host_ip(addr, ip, 256);
	return std::string((strncmp(ip, "::ffff:", 7) == 0) ? (ip + 7) : (ip));
}

const char* addr(ENetAddress* addr)
{
	static char buf[256];
	snprintf(buf, 256, "%s:%hu", addr_ip(addr).c_str(), addr->port);
	return buf;
}

char to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

std::string urlencode(std::string in)
{
	std::string out;

	for (const char& c : in)
	{
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' ||
			c == '!' || c == '*' || c == '\'' || c == '(' || c == ')')
		{
			out += c;
		}
		else
		{
			out += "%";
			out += to_hex(c >> 4);
			out += to_hex(c);
		}
	}
	return out;
}

void server_config_t::load_cmdline(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		std::string v = argv[i];

		/**/ if (v == "-f")
		{
			if ((i + 1) >= argc)
				break;

			this->logfile = argv[++i];
		}
		else if (v == "-p")
		{
			if ((i + 1) >= argc)
				break;

			try { this->port = std::stoi(argv[++i]); }
			catch (...) {}
		}
		else if (v == "-sqlite")
		{
			if ((i + 1) >= argc)
				break;

			this->sqlite_filename = argv[++i];
		}
	}
}

bool redirect_stdout(const char* filename)
{
	return freopen(filename, "a", stdout) != NULL;
}

void __printf_stdout(const char* fmt, ...)
{
	time_t time_ = time(0);
	struct tm tm_;
	fill_timeinfo(&tm_);

	fprintf(stdout, "[%02d:%02d:%02d] ", tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

	va_list va;
	va_start(va, fmt);
	vfprintf(stdout, fmt, va);
	va_end(va);

	fprintf(stdout, "\n");
	fflush(stdout);
}

void fill_timeinfo(tm* tm)
{
	time_t time_ = time(NULL);
#ifdef _WINDOWS
	localtime_s(tm, &time_);
#else
	localtime_r(&time_, tm);
#endif
}

void format_nick(std::string& out, user_t* user, user_t* target)
{
	out = user->nick_c();
	out += " (";
	out += std::to_string(user->m_status);
	out += ")";

	bool info = target && target->m_status > 3;

	if (user->m_watching && (info || user->m_status < 4))
		out += " \uf11a";

	if (user->m_hideme && info)
		out += " \uf070";
}

void trim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

std::string format_out(std::string_view in, bool remove_color)
{
	std::string result;

	// filter: size, bad chars
	for (auto &c : in)
	{
		if ((uint8_t)c < 32)
			continue;
		
		result += c;
		
		if (result.size() > 255)
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
		{":)", "\uf118"}, {">:(","\uf556"}, {":|", "\uf11a"}, {"(f)","\uf024"},
		{";)", "\uf58c"}, {":(", "\uf119"}, {":D", "\uf599"}, {":'(","\uf5b4"},
		{":o", "\uf5c2"}, {":p", "\uf58a"}, {"=)", "\uf581"}, {"xD", "\uf586"},
		{"(r)","\uf25d"}, {"(c)","\uf1f9"}, {"(+)","\uf164"}, {"(-)","\uf165"},
		{"(*)","\uf005"}, {"($)","\uf3d1"}, {"(%)","\uf3a5"}, {"<3", "\uf004"},
		{"0_0","\uf579"}, {":*", "\uf598"}, {"(t)","\uf2ed"}, {":P", "\uf589"},
		{"YoY","\uf5b3"}, {":B", "\uf57f"},
	};

	for (auto it : smiles)
		string_replace_all(result, it[0], it[1]);

	return result;
}

bool nick_is_valid(std::string_view nick, bool check_min)
{
	if ((nick.size() < 5 && check_min) || nick.size() > 24)
		return false;

	for (char c : nick)
		if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
			return false;
	return true;
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
