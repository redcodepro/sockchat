#include "main.h"

class user_t
{
public:
	peer_t		m_peer;
	std::string m_addr;
	bool		m_watching;
	bool		m_rainbow;
	bool		m_hideme;
	bool		m_motd;
	std::string m_displayname;

	int			m_id;
	std::string	m_prefix;
	std::string	m_nick;
	int			m_status;
	color_t		m_color;

	int			m_pm_id;
	int			m_room_id;

	user_t(peer_t peer);

	void udn(); // update display name

	void login(userdata_t* ud);
	void logout();

	void send(packet_t* packet);
	void send_auth(const std::string& auth);
	void send_erase(const std::string& text);
	void send_erase(id_t id);
	void send_unreaded(int count);

	void send_notify();

	void AddChat(color_t color, const char* fmt, ...);

	void OnConnect();
	void OnDisconnect();
	void OnAuth(const std::string& key);
	void OnPacket(packet_t* packet);
	void OnChat(const std::string& text);
	void OnCommand(const std::string& text);

	void set_color(color_t color);
	void set_nick(const std::string& nick);
	void set_prefix(const std::string& prefix);
	void set_status(int status);

	const char* nick() { return m_displayname.c_str() + 8; }
	const char* nick_c() { return m_displayname.c_str(); }
};
