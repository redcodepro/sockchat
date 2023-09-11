#include "main.h"

typedef struct entry_t
{
	id_t		m_id;
	time_t		m_time;
	color_t		m_color;
	std::string	m_text;
	int			m_status;

	entry_t() {}
	entry_t(id_t id, color_t color, const std::string& text, int status = 1)
		: m_id(id),
		m_time(time(0)),
		m_color(color),
		m_text(text),
		m_status(status)
	{}
} entry_t;
typedef std::deque<entry_t> entries_t;

class chat_handler_t
{
private:
	entries_t	m_lines;
public:
	void		send_history(user_t* user);
	int			clear(const char* text = nullptr);
	int			clear_id(id_t id = 0);

	void		push(const entry_t& entry);
	void		send(const entry_t& entry);

	void		pushf(int status, id_t id, color_t color, const char* fmt, ...);
	void		sendf(int status, id_t id, color_t color, const char* fmt, ...);
};
