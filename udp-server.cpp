#include "main.h" // ÙŦF-8

bool udpserver_t::bind(const char* ipv4, uint16_t port)
{
	m_addr.host = ENET_HOST_ANY;
	m_addr.port = 7778;

	m_server = enet_host_create(&m_addr, CHAT_MAX_CLIENTS, 2, 0, 0);

	return (m_server != nullptr);
}

void udpserver_t::exec()
{
	srand(time(0));
	srand(time(0) + rand());
	m_crypt.init(rand());

	_printf("[info] Server running at: %s", addr(&m_addr));

	ENetEvent ev;
	while (enet_host_service(m_server, &ev, 500) >= 0)
	{
		switch (ev.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			{
				char ip[256];
				enet_address_get_host_ip(&ev.peer->address, ip, 256);

				if (db.find_banip(ip))
				{
					enet_peer_disconnect_now(ev.peer, 0);
					break;
				}

				if (user_t* user = new user_t(ev.peer))
				{
					ev.peer->data = user;
					m_users.insert(std::pair(ev.peer, user));

					opacket_t packet(id_chat_crypto);
					packet.write<unsigned int>(m_crypt.seed());
					enet_peer_send(ev.peer, 0, packet.to_enet());
				}
			}
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			{
				if (user_t* user = (user_t*)ev.peer->data)
				{
					ipacket_t packet(ev.packet);
					user->OnPacket(&packet);
				}

				enet_packet_destroy(ev.packet);
			}
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			{
				if (user_t* user = (user_t*)ev.peer->data)
				{
					m_users.erase(ev.peer);
					user->OnDisconnect();
					delete user;
				}
			}
			break;

		case ENET_EVENT_TYPE_NONE:
			break;
		}

		send_online(); // ????
	}

	_printf("[info] Server stopped");
}

void udpserver_t::Broadcast(opacket_t* packet, int level)
{
#if (CHAT_SEND_NOAUTH == 1)
	if (level == 1)
		level = 0;
#endif
	m_crypt.encrypt(packet);

	ENetPacket* ep = packet->to_enet();
	for (auto& it : m_users)
	{
		peer_t peer = it.first;
		user_t* user = it.second;

		if (user->m_status < level)
			continue;

		enet_peer_send(peer, 0, ep);
	}
}

void udpserver_t::AddEventGlobal(const std::string& text)
{
	opacket_t packet(id_chat_event);
	packet.write_string(text);
	Broadcast(&packet);
}

void udpserver_t::KickUser(user_t* user, bool send_closed)
{
	enet_peer_disconnect_later(user->m_peer, 0);
}

void udpserver_t::MakeBan(user_t* user)
{
	db.set_status(user->m_id, 0);
	KickUser(user);
}

void udpserver_t::MakeBanIP(user_t* user)
{
	//db.add_banip(0);
	MakeBan(user);
}

void udpserver_t::SendPM(user_t* src, user_t* dst, const char* message)
{
	src->AddChat(0xFFF7F488, "[Я >> %s] {ffffff}%s", dst->m_nick.c_str(), message);
	dst->AddChat(0xFFF7F488, "[%s >> Я] {ffffff}%s", src->m_nick.c_str(), message);
	src->m_xid = dst->m_id;
	dst->m_xid = src->m_id;
	dst->send_notify();

	_printf("[private] [%s >> %s] %s", src->m_nick.c_str(), dst->m_nick.c_str(), message);
}

void udpserver_t::NotifyAll(int level)
{
	/*packet_t packet(id_notify_play);
	for (auto& it : m_users)
	{
		user_t* user = it.second;

		if (user->m_status < level)
			continue;

		if (user->m_notify == false)
			continue;

		if (user->m_watching == true)
			continue;

		sendto(&packet, &user->m_addr);
	}*/
}

void udpserver_t::NotifySet(const std::string& url)
{
	opacket_t packet(id_notify_set_url);
	packet.write_string(url);
	Broadcast(&packet);
}

void udpserver_t::NotifyPlay(const std::string& url)
{
	opacket_t packet(id_notify_play_url);
	packet.write_string(url);
	Broadcast(&packet);
}
