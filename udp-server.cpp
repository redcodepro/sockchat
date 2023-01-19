#include "main.h" // ÙŦF-8

bool udpserver_t::bind(const char* ipv4, uint16_t port)
{
	in_addr_t addr = inet_addr(ipv4);
	if (addr == INADDR_NONE)
		return false;

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = addr;
	m_addr.sin_port = htons(port);

	SOCKET_CLOSE(m_socket);

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
		return false;

	if (::bind(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) != 0)
		return false;

	return true;
}

ssize_t udpserver_t::recvfrom(packet_t* packet, sockaddr_in* from)
{
	socklen_t fromlen = sizeof(from);
	return ::recvfrom(
		m_socket,
		packet, sizeof(packet_t),
		MSG_DONTWAIT,
		(sockaddr*)from, (socklen_t*)&fromlen
	);
}

ssize_t udpserver_t::sendto(packet_t* packet, sockaddr_in* to)
{
	return ::sendto(
		m_socket,
		packet, (packet->len + sizeof(packet_header_t)),
		0,
		(sockaddr*)to, sizeof(*to)
	);
}

void udpserver_t::init_user(packet_t* packet, sockaddr_in* from)
{
	if (packet->ReadString() != "Sockchat/1.1")
		return;

	user_t* user = new user_t(from, m_time);
	if (user == nullptr)
		return;

	packet_t out(id_conn_init);
	out.Write<uint16_t>(-1); // packet cookie // not used
	out.Write<uint32_t>(20000);
	out.Write<uint32_t>(m_crypt.seed());
	user->send_r(&out);

//	if (m_notify_url.size())
//		user->send_notify_set(m_notify_url);

	m_users.insert(std::pair(user->m_udpid, user));

	_printf("[connect] Connect: %s", addr(from));
	on_udn();
}

void udpserver_t::on_recv(packet_t* packet, sockaddr_in* from)
{
	auto it = m_users.find(udpid_t(from));
	if (it != m_users.end())
	{
		it->second->OnPacket(packet);
		return;
	}

	if (packet->id == id_conn_request)
	{
		auto res = m_banned.find(from->sin_addr.s_addr);
		if (res == m_banned.end())
		{
			init_user(packet, from);
		}
		else
		{
			packet_t packet(id_conn_banned);
			sendto(&packet, from);
			_printf("[connect] Connection banned: %s", addr(from));
		}
	}
}

void udpserver_t::on_idle()
{
	m_time = time(0);

	free_users();

	if (m_users.empty())
		return;

	ping_users();
	send_online();
}

void udpserver_t::process()
{
	srand(time(0));
	srand(time(0) + rand());
	m_crypt.init(rand());

	_printf("[info] Server running at: %s", addr(&m_addr));

	while (m_active)
	{
		on_idle();

		packet_t packet;
		sockaddr_in from;
		ssize_t rc = recvfrom(&packet, &from);
		if (rc == SOCKET_ERROR)
		{
			if (errno == EAGAIN)
			{
				Sleep(10);
				continue;
			}
			_printf("[info] Server has crashed");
			return;
		}

		if (rc < sizeof(packet_header_t) || rc != (packet.len + sizeof(packet_header_t)))
		{
			_printf("[info] bad packet");
			continue;
		}

		on_recv(&packet, &from);
	}
	_printf("[info] Server stopped");
}

void udpserver_t::ping_users()
{
	// update period: 1s
	if (m_time == m_lpu)
		return;
	m_lpu = m_time;

	for (auto& it : m_users)
	{
		user_t* user = it.second;
		time_t elapsed = (m_time - user->m_lpr);

		if (user->m_rainbow)
		{
			user->m_color = create_color();
			user->udn();
		}

		if (elapsed < 10)
			continue;

		if (elapsed < 20)
		{
			user->send_ping();
			continue;
		}

//		_printf("[info] User %s didn`t respond.", addr(&user->m_addr));
		m_free.push_back(it.first);
	}
}

void udpserver_t::free_users()
{
	if (m_free.empty())
		return;

	for (auto& it : m_free)
	{
		user_t* user = m_users.at(it);
		m_users.erase(it);
		user->OnDisconnect();
		_printf("[connect] Disconnect: %s", addr(&user->m_addr));
		on_udn();
		delete user;
	}
	m_free.clear();
}

void udpserver_t::exec()
{
	m_active = true;
	process();
	m_active = false;
}

void udpserver_t::Broadcast(packet_t* packet, int level)
{
	m_crypt.encrypt(packet);
	for (auto& it : m_users)
	{
		user_t* user = it.second;

		if (user->m_status < level)
			continue;

		sendto(packet, &user->m_addr);
	}
}

void udpserver_t::AddChatGlobal(id_t id, color_t color, const char* fmt, ...)
{
	char buf[512];

	va_list va;
	va_start(va, fmt);
	int count = vsnprintf(buf, 512, fmt, va);
	va_end(va);

	if (count < 0) return;
	if (count > 512) count = 512;

	chat.push(entry_t(id, color, buf, 1));
}

void udpserver_t::AddEventGlobal(const std::string& text)
{
	packet_t packet(id_event);
	packet.WriteString(text);
	Broadcast(&packet);
}

void udpserver_t::KickUser(user_t* user)
{
	user->send_kicked();
	m_free.push_back(user->m_udpid);
}

void udpserver_t::MakeBan(user_t* user)
{
	db.set_status(user->m_id, 0);
	KickUser(user);
}

void udpserver_t::MakeBanIP(user_t* user)
{
	m_banned.insert(user->m_addr.sin_addr.s_addr);
	db.add_banip(inet_ntoa(user->m_addr.sin_addr));
	MakeBan(user);
}

void udpserver_t::SendPM(user_t* src, user_t* dst, const char* message)
{
	src->AddChat(0xFFF7F488, "[Я >> %s] {ffffff}%s", dst->m_nick.c_str(), message);
	dst->AddChat(0xFFF7F488, "[%s >> Я] {ffffff}%s", src->m_nick.c_str(), message);
	src->m_xid = dst->m_id;
	dst->m_xid = src->m_id;
}

void udpserver_t::NotifyAll(int level)
{
	packet_t packet(id_notify_play);
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
	}
}

void udpserver_t::NotifySet(const std::string& url)
{
//	m_notify_url = url;

	packet_t out(id_notify_set_url);
	out.WriteString(url);

	m_crypt.encrypt(&out);

	for (auto& it : m_users)
		it.second->send_r(&out);
}

void udpserver_t::NotifyPlay(const std::string& url)
{
	packet_t out(id_notify_play_url);
	out.WriteString(url);

	m_crypt.encrypt(&out);

	for (auto& it : m_users)
		it.second->send_r(&out);
}
