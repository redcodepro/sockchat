#include "main.h" // ÙŦF-8

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

bool nick_is_valid(const std::string& nick, bool check_min)
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
	return cfg.audio_folder + f;
}
