#include "main.h"

class user_t
{
public:
	peer_t		m_peer;
	bool		m_watching;
	bool		m_rainbow;
	bool		m_hideme;
	bool		m_notify;
	std::string m_displayname;

	int			m_id;
	std::string	m_prefix;
	std::string	m_nick;
	int			m_status;
	color_t		m_color;

	int			m_pm_id;
	int			m_room_id;

	user_t(peer_t peer)
		: m_peer(peer), m_watching(0), m_rainbow(0), m_hideme(0),
		m_notify(1), m_displayname("{ffffff}[-1]"), m_id(-1),
		m_status(0), m_color(-1), m_pm_id(-1), m_room_id(-1)
	{}

	void udn(); // update display name

	void login(userdata_t* ud);
	void logout();

	void send(packet_t* packet);
	void send_auth(const std::string& auth);
	void send_erase(const std::string& text);
	void send_erase(id_t id);
	void send_hudtext(const std::string& text);
	void send_unreaded(int count);

	void send_notify();
	void send_notify_set(const std::string& url);
	void send_notify_play(const std::string& url);

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
