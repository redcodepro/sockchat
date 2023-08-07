#include "sc_types.h"
#include "md5.h"
#include <stdio.h>

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
	do
	{
		char c = (char)(rand() & 0xFF);
		if ((c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z'))
			result.push_back(c);
	} while (result.size() != length);
	return result;
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
	return freopen(filename, "w", stdout) != NULL;
}
