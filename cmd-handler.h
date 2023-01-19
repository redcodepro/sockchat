#include "main.h"

typedef struct cmd_t
{
	int			status;
	const void* func;
	const char* args;
	const char* hint;
} cmd_t;

typedef std::unordered_map<std::string, cmd_t*> cmdlist_t;

class cmd_handler_t
{
private:
	cmdlist_t m_cmds;
	void send_usage(user_t* user, cmdlist_t::iterator it);
	void add_cmd(std::string name, cmd_t* desc);
public:
	void add(std::vector<std::string> names, cmd_t* desc);
	void run(user_t* user, const std::string& cmd, const std::string& arg);
	std::size_t get_count() { return m_cmds.size(); }
};
