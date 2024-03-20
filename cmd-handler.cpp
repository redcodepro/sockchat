#include "main.h" // ÙŦF-8

template<int C>
struct args_t
{
	void* args[C];
};

class va_runner
{
private:
	std::string	sscanf_fmt;
	void*		sscanf_args[4];
	args_t<4>	args;
	size_t		args_count;

	void _init(const char* desc)
	{
		memset(sscanf_args, 0, sizeof(sscanf_args));
		memset(&args, 0, sizeof(args));

		args_count = strlen(desc);

		for (size_t i = 0; i < args_count; ++i)
		{
			switch (desc[i])
			{
			case 'c': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%c"; break;
			case 'd': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%d"; break;
			case 'u': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%u"; break;
			case 'o': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%o"; break;
			case 'x': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%x"; break;
			case 'f': sscanf_args[i] = &args.args[i]; sscanf_fmt += "%f"; break;

			case 's':
				sscanf_args[i] = new char[256]; sscanf_fmt += "%256s";
				args.args[i] = sscanf_args[i];
				break;
			case '*':
				sscanf_args[i] = new char[256]; sscanf_fmt += " %256[^\n]";
				args.args[i] = sscanf_args[i];
				break;
			}
			if (sscanf_args[i] == nullptr)
			{
				args_count = 0;
				break;
			}
		}
//		_printf("sscanf_fmt=\"%s\"; args_count=%d;", sscanf_fmt.c_str(), args_count);
	}
	void _free()
	{
		for (int i = 0; i < 4; i++)
		{
			if (sscanf_args[i] != nullptr
				&& sscanf_args[i] != &args.args[i])
				delete[] sscanf_args[i];
			sscanf_args[i] = nullptr;
		}
	}
	template<int C>
	bool _exec(user_t* user, const void* func, const char* text)
	{
		if (sscanf(text, sscanf_fmt.c_str(), *(args_t<C>*)sscanf_args) != C)
			return false;

		((void(*)(user_t*, args_t<C>))func)(user, *(args_t<C>*)(&args));

		return true;
	}
public:
	va_runner(const char* desc) : args_count(0) { _init(desc); }
	~va_runner() { _free(); }
	
	bool _run(const void* func, user_t* user, const char* args)
	{
		switch (args_count)
		{
		case 1: return _exec<1>(user, func, args); break;
		case 2: return _exec<2>(user, func, args); break;
		case 3: return _exec<3>(user, func, args); break;
		case 4: return _exec<4>(user, func, args); break;
		}
		return false;
	}
};

void cmd_handler_t::send_usage(user_t* user, cmdlist_t::iterator it)
{
	user->AddChat(0xFFDB0000, "Используйте: {ffffff}/%s %s", it->first.c_str(), it->second->hint);
}

void cmd_handler_t::add_cmd(std::string name, cmd_t* desc)
{
	std::pair<std::string, cmd_t*> pair;
	pair.first = name;
	pair.second = desc;
	m_cmds.insert(pair);
}

void cmd_handler_t::add(std::vector<std::string> names, cmd_t* desc)
{
	for (auto& it : names)
		add_cmd(it, desc);
}

void cmd_handler_t::run(user_t* user, const std::string& cmd, const std::string& arg)
{
	auto res = m_cmds.find(cmd);
	if (res == m_cmds.end()) {
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Неизвестная команда.");
		return;
	}

	cmd_t* _cmd = res->second;

	if (user->m_status == 0) {
		if (_cmd->status != 0) {
			user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Требуется авторизация!");
			return;
		}
	}
	else {
		if (_cmd->status == 0 || user->m_status < _cmd->status) {
			user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Нет доступа!");
			return;
		}
	}

	if (_cmd->args[0] == '\0')
	{
		((void(*)(user_t*))_cmd->func)(user);
		return;
	}

	if (_cmd->args[0] == '*')
	{
		if (arg.empty())
			send_usage(user, res);
		else
			((void(*)(user_t*, const char*))_cmd->func)(user, arg.c_str());
		return;
	}

	va_runner va(_cmd->args);
	if (!va._run(_cmd->func, user, arg.c_str()))
		send_usage(user, res);
}
