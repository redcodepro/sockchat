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

	_printf("[info] Server running at %s", addr(&m_addr));

	while (true)
	{
		ENetEvent ev;

		int result = enet_host_service(m_host, &ev, 500);
		if (result < 0)
		{
			static int count = 0;
			if (++count > 100)
				break;

			_printf("[error] Service failed %d times.", count);
		}

		handle_event(&ev);

		static time_t prev = 0;
		time_t now = time(0);
		if (prev != now)
		{
			for (auto& [peer, user] : m_users)
				if (user->m_rainbow)
				{
					user->m_color = create_color();
					user->udn();
				}
			prev = now;
		}
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

				packet_t packet(id_chat_init);
				packet.write<enet_uint32>(m_crypt.seed());
				packet.write_string(m_name);
				enet_peer_send(ev->peer, 0, packet.get());
				enet_peer_timeout(ev->peer, 0, 5000, 15000);
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
				on_count();
			}
		}
		break;

	case ENET_EVENT_TYPE_RECEIVE:
		{
			if (user_t* user = (user_t*)ev->peer->data)
			{
				packet_t packet(ev->packet);
				user->OnPacket(&packet);
			}
		}
		break;
	}
}

void udpserver_t::Broadcast(packet_t* packet, int level, enet_uint8 channel)
{
	m_crypt.encrypt(packet);
	for (auto& [peer, user] : m_users)
		if (user->m_status >= level)
			enet_peer_send(peer, channel, packet->get());
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

void udpserver_t::SendPM(user_t* src, user_t* dst, const std::string& text)
{
	src->AddChat(0xFFF7F488, "[Я >> %s[%d]] {ffffff}%s", dst->m_nick.c_str(), dst->m_id, text.c_str());
	dst->AddChat(0xFFF7F488, "[%s[%d] >> Я] {ffffff}%s", src->m_nick.c_str(), dst->m_id, text.c_str());
	src->m_pm_id = dst->m_id;
	dst->m_pm_id = src->m_id;
	dst->send_notify();
}

void udpserver_t::on_count()
{
	int users = 0;
	int total = 0;

	for (auto& [peer, user] : m_users)
	{
		if (user->m_status)
		{
			if (user->m_hideme)
				continue;

			users += 1;
		}
		total += 1;
	}

	char buf[64];
	snprintf(buf, 64, "\uf007 %d / %d | Онлайн:", users, total);
	m_htHeader = buf;
	
	packet_t packet(id_ht_header_set);
	packet.write_string(m_htHeader);
	server.Broadcast(&packet);
}

void udpserver_t::on_nick(user_t* user)
{
	if (user->m_hideme) {
		on_hide(user);
		return;
	}
	std::string str;
	format_nick(str, user);

	packet_t packet(id_ht_entry_insert);
	packet.write<id_t>(user->m_id);
	packet.write_string(str);
	server.Broadcast(&packet);
}

void udpserver_t::on_hide(user_t* user)
{
	packet_t packet(id_ht_entry_remove);
	packet.write<id_t>(user->m_id);
	server.Broadcast(&packet);
}

void udpserver_t::send_online(user_t* _dst)
{
	online_foreach([](user_t* user, const std::string& str, user_t* dst)
	{
		if (user->m_hideme)
			return;
		packet_t packet(id_ht_entry_insert);
		packet.write<id_t>(user->m_id);
		packet.write_string(str);
		dst->send(&packet);
	}, _dst);
}

void udpserver_t::online_foreach(void(*func)(user_t* user, const std::string& str, user_t* dst), user_t* dst, bool info)
{
	for (auto& [peer, user] : m_users)
	{
		if (user->m_status == 0)
			continue;

		std::string str;
		format_nick(str, user, info ? dst : nullptr);
		func(user, str, dst);
	}
}
