#include "main.h"

class user_t
{
public:
	udpid_t		m_udpid;
	sockaddr_in	m_addr;
	time_t		m_lpr; // last packet received
	bool		m_watching;
	bool		m_rainbow;
	bool		m_hideme;
	bool		m_notify;
	std::string m_displayname;
	std::string m_displayname_c;

	int			m_id;
	std::string	m_prefix;
	std::string	m_nick;
	int			m_status;
	color_t		m_color;

	int			m_xid;

	user_t(sockaddr_in* addr, time_t t)
		: m_udpid(addr), m_addr(*addr), m_lpr(t),
		m_watching(0), m_rainbow(0), m_hideme(0), m_notify(1),
		m_id(-1), m_status(0), m_color(-1), m_xid(-1)
	{}

	void udn(); // update display name

	void login(userdata_t* ud);
	void logout();

	void send_ping();
	void send_kicked();
	void send_watching();

	void send_auth(const std::string& auth);
	void send_event(const std::string& text);
	void send_hudtext(const std::string& text);
	void send_unreaded(int count);

	void send_notify();
	void send_notify_set(const std::string& url);
	void send_notify_play(const std::string& url);

	int  send_r(packet_t* packet);	// send packet w/o encryption
	int  send(packet_t* packet);	// send packet

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

	const char* nick() { return m_displayname.c_str(); }
	const char* nick_c() { return m_displayname_c.c_str(); }
};
