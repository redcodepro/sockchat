#include "main.h" // ÙŦF-8

typedef std::unordered_map<peer_t, user_t*> udpusers_t;

class udpserver_t
{
	friend class user_t;
private:
	ENetHost*	m_server = nullptr;
	ENetAddress m_addr = { 0 };
	udpusers_t	m_users;
	udpcrypt_t	m_crypt;
	time_t		m_lou; // last online update

	void send_online();
public:
	udpserver_t() { enet_initialize(); };
	~udpserver_t() { enet_deinitialize(); };

	bool bind(const char* ipv4, uint16_t port);
	void exec();

	//////////////////////////////////////////

	void Broadcast(opacket_t* packet, int level = 0);
	void KickUser(user_t* user, bool send_closed = true);
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
	void on_udn() { m_lou = 0; }
};
