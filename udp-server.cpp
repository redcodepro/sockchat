#include "main.h" // ÙŦF-8

bool udpserver_t::init(server_config_t* cfg)
{
	m_name = cfg->name;

	m_addr.host = ENET_HOST_ANY;
	m_addr.port = static_cast<enet_uint16>(cfg->port);

	m_host = enet_host_create(&m_addr, cfg->slots, 2, 0, 0);

	return (m_host != nullptr);
}

void udpserver_t::exec()
{
	_srand();
	m_crypt.init(rand());

	_printf("[info] Server running at 0.0.0.0:%hu", m_addr.port);

	while (true)
	{
		ENetEvent ev;

		int result = enet_host_service(m_host, &ev, 500);
		if (result < 0)
		{
			static int count = 0;
			if (++count > 100)
				break;

			_printf("[error] Service failed %d times. uc: %d", count, m_users.size());
		}

		handle_event(&ev);

		send_online(); // ????
	}

	_printf("[info] Server stopped");
}

void udpserver_t::handle_event(ENetEvent* ev)
{
	switch (ev->type)
	{
	case ENET_EVENT_TYPE_NONE:
		break;

	case ENET_EVENT_TYPE_CONNECT:
		{
			if (db.find_banip(addr_ip(&ev->peer->address)))
			{
				enet_peer_disconnect_later(ev->peer, 1);
				break;
			}

			if (user_t* user = new user_t(ev->peer))
			{
				ev->peer->data = user;
				m_users.insert(std::pair(ev->peer, user));

				opacket_t packet(id_chat_init);
				packet.write<unsigned int>(m_crypt.seed());
				packet.write_string(m_name);
				enet_peer_send(ev->peer, 0, packet.to_enet());
			}
		}
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
	case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
		{
			if (user_t* user = (user_t*)ev->peer->data)
			{
				m_users.erase(ev->peer);
				user->OnDisconnect();
				delete user;
			}
		}
		break;

	case ENET_EVENT_TYPE_RECEIVE:
		{
			if (user_t* user = (user_t*)ev->peer->data)
			{
				ipacket_t packet(ev->packet);
				user->OnPacket(&packet);
			}

			enet_packet_destroy(ev->packet);
		}
		break;
	}
}

void udpserver_t::Broadcast(opacket_t* packet, int level)
{
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
	db.add_banip(addr_ip(&user->m_peer->address));
	MakeBan(user);
}

void udpserver_t::SendPM(user_t* src, user_t* dst, const char* message)
{
	src->AddChat(0xFFF7F488, "[Я >> %s[%d]] {ffffff}%s", dst->m_nick.c_str(), dst->m_id, message);
	dst->AddChat(0xFFF7F488, "[%s[%d] >> Я] {ffffff}%s", src->m_nick.c_str(), dst->m_id, message);
	src->m_xid = dst->m_id;
	dst->m_xid = src->m_id;
	dst->send_notify();
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
