#include "main.h"

void chat_handler_t::push(const entry_t& entry)
{
	m_lines.push_back(entry);

	if (m_lines.size() > 100)
		m_lines.pop_front();

	send(entry);
}

void chat_handler_t::send_history(user_t* user)
{
	for (auto& it : m_lines)
	{
		if (user->m_status < it.m_status)
			continue;
		
		packet_t out(id_message);
		out.Write<id_t>(it.m_id);
		out.Write<time_t>(it.m_time);
		out.Write<color_t>(it.m_color);
		out.WriteString(it.m_text);

		user->send(&out);
	}
	user->send_unreaded(1);
}

void chat_handler_t::clear(const char* text)
{
	if (text == nullptr || text[0] == '\0')
	{
		m_lines.clear();
		return;
	}

	for (std::size_t i = 0; i < m_lines.size(); ++i)
	{
		if (strstr(m_lines[i].m_text.c_str(), text))
		{
			m_lines.erase(m_lines.begin() + i);
			i--;
		}
	}
}

int chat_handler_t::clear_id(id_t id)
{
	int c = 0;
	for (std::size_t i = 0; i < m_lines.size(); ++i)
	{
		if (m_lines[i].m_id == id)
		{
			m_lines.erase(m_lines.begin() + i);
			i--;
			c++;
		}
	}
	return c;
}

void chat_handler_t::send(const entry_t& entry)
{
	_printf("[chat] %s", entry.m_text.c_str());

	packet_t out(id_message);
	out.Write<id_t>(entry.m_id);
	out.Write<time_t>(entry.m_time);
	out.Write<color_t>(entry.m_color);
	out.WriteString(entry.m_text);

	server.Broadcast(&out, entry.m_status);
	server.NotifyAll(entry.m_status);
}
