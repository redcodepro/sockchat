#include "main.h" // ÙŦF-8

typedef std::deque<udpid_t> udpids_t;
typedef std::unordered_map<udpid_t, user_t*> udpusers_t;
typedef std::unordered_set<in_addr_t> bannedips_t;

class udpserver_t
{
	friend class user_t;
private:
	SOCKET		m_socket;
	sockaddr_in	m_addr;
	
	udpusers_t	m_users;
	udpids_t	m_free;
	bannedips_t	m_banned;

	udpcrypt_t	m_crypt;
	time_t		m_time;
	time_t		m_lou; // last online update
	time_t		m_lpu; // last ping users
//	std::string	m_notify_url;

	bool		m_active;

	// internal functions
	ssize_t		recvfrom(packet_t* packet, sockaddr_in* from);
	ssize_t		sendto(packet_t* packet, sockaddr_in* to);

	void		process();
	void		on_recv(packet_t* packet, sockaddr_in* from);
	void		on_idle();

	void init_user(packet_t* packet, sockaddr_in* from);


	void ping_users();
	void free_users();
	void send_online();
public:
	udpserver_t() : m_socket(INVALID_SOCKET), m_time(0), m_lou(0), m_lpu(0), m_active(0) {};
	~udpserver_t() {};

	bool bind(const char* ipv4, uint16_t port);
	void exec();

	//////////////////////////////////////////

	void Broadcast(packet_t* packet, int level = 0);
	void AddChatGlobal(id_t id, color_t color, const char* fmt, ...);
	void AddEventGlobal(const std::string& text);
	void KickUser(user_t* user);
	void MakeBan(user_t* user);
	void MakeBanIP(user_t* user);
	void SendPM(user_t* src, user_t* dst, const char* message);
	void NotifyAll(int level = 0);
	void NotifySet(const std::string& url);
	void NotifyPlay(const std::string& url);

	user_t* find_user(int id)
	{
		if (id > 0)
			for (auto& it : m_users)
				if (it.second->m_id == id)
					return it.second;
		return nullptr;
	}
	void add_banip(in_addr_t addr) { m_banned.insert(addr); }
	void on_udn() { m_lou = 0; }
};
