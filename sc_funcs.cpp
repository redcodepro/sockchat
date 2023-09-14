#include "main.h"
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
		else if (v == "-audio")
		{
			if ((i + 1) >= argc)
				break;

			this->audio_folder = argv[++i];
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
	localtime_r(&time_, &tm_);

	fprintf(stdout, "[%02d:%02d:%02d] ", tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

	va_list va;
	va_start(va, fmt);
	vfprintf(stdout, fmt, va);
	va_end(va);

	fprintf(stdout, "\n");
	fflush(stdout);
}

packet_t* create_audio_packet(packet_id id, const std::string& filename)
{
	std::size_t p = filename.find_last_of('/');
	std::string f = (p == std::string::npos ? filename : filename.substr(p + 1)) + ".mp3";
	std::string n = "/var/www/html/audio/" + f;

	std::ifstream ifs(n, std::ios::binary);
	std::ostringstream oss(std::ios::binary);
	oss << ifs.rdbuf();
	std::string data = oss.str();

	if (data.size())
	{
		packet_t* packet = new packet_t(id);
		packet->write_string(data);
		return packet;
	}
	return nullptr;
}
