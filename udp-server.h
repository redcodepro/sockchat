﻿#include "main.h" // ÙŦF-8

typedef std::unordered_map<peer_t, user_t*> udpusers_t;

class udpserver_t
{
	friend class user_t;
private:
	ENetHost*	m_host = nullptr;
	ENetAddress m_addr = { 0 };
	udpusers_t	m_users;
	udpcrypt_t	m_crypt;
	std::string	m_name;
	std::string m_htHeader;
	std::string m_htFooter;

	void handle_event(ENetEvent* ev);
public:
	udpserver_t() { enet_initialize(); };
	~udpserver_t() { enet_deinitialize(); };

	bool init(server_config_t* cfg);
	void exec();

	//////////////////////////////////////////

	void Broadcast(packet_t* packet, int level = 0, enet_uint8 channel = 0);
	void KickUser(user_t* user, bool send_closed = true);
	void MakeBan(user_t* user);
	void MakeBanIP(user_t* user);
	void SendPM(user_t* src, user_t* dst, const std::string& text);

	user_t* find_user(int id)
	{
		if (id > 0)
			for (auto& it : m_users)
				if (it.second->m_id == id)
					return it.second;
		return nullptr;
	}

	void on_count();
	void on_nick(user_t* user);
	void on_hide(user_t* user);
	void send_online(user_t* user);

	std::string& get_header() { return m_htHeader; };
	std::string& get_footer() { return m_htFooter; };
	void online_foreach(void(*func)(user_t* user, const std::string& str, user_t* dst), user_t* dst, bool info = false);
};
