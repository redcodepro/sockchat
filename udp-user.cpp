#include "main.h" // ÙŦF-8

user_t::user_t(peer_t peer)
	: m_peer(peer), m_watching(false), m_rainbow(false), m_hideme(false), m_motd(false),
	m_displayname("{ffffff}[-1]"), m_id(-1), m_status(0), m_color(-1), m_pm_id(-1), m_room_id(-1)
{
	m_addr = addr(&peer->address);
}

void user_t::udn()
{
	char buf[2048];
	snprintf(buf, 2048, "{%06x}%s%s[%d]", (m_color & 0xFFFFFF), m_prefix.c_str(), m_nick.c_str(), m_id);
	m_displayname = buf;

	if (m_status != 0) server.on_nick(this);
}

void user_t::login(userdata_t* ud)
{
	if (ud->m_status == 0)
	{
		server.KickUser(this);
		return;
	}

	if (server.find_user(ud->m_id))
	{
		AddChat(0xFFDB0000, "[Ошибка] {ffffff}ЗАНЯТО НАХуй!1!! Раздвоение личности detected!111");
		return;
	}

	m_id = ud->m_id;
	m_nick = ud->m_nick;
	m_color = ud->m_color;
	m_prefix = ud->m_prefix;
	m_status = ud->m_status;
	user_t::udn();

	std::string restore = create_key(64);
	db.set_restore(m_id, restore);
	user_t::send_auth(restore);

	OnConnect();

	server.on_count();
	server.send_online(this);
}

void user_t::logout()
{
	OnDisconnect();

	m_status = 0;
	m_prefix = "";
	m_color = -1;
	m_nick = "";
	m_id = -1;
	user_t::udn();

	user_t::send_auth("");

	server.on_count();
}

void user_t::send(packet_t* packet)
{
	server.m_crypt.encrypt(packet);
	enet_peer_send(m_peer, 0, packet->get());
}

void user_t::send_auth(const std::string& auth)
{
	packet_t packet(id_chat_auth);
	packet.write_string(auth);
	user_t::send(&packet);
}

void user_t::send_unreaded(int count)
{
	packet_t packet(id_chat_unreaded);
	packet.write<int>(count);
	user_t::send(&packet);
}

void user_t::send_notify()
{
	packet_t packet(id_notify);
	user_t::send(&packet);
}

void user_t::send_erase(const std::string& text)
{
	packet_t packet(id_chat_erase);
	packet.write_string(text);
	user_t::send(&packet);
}

void user_t::send_erase(id_t id)
{
	packet_t packet(id_chat_erase);
	packet.write<id_t>(id);
	user_t::send(&packet);
}

void user_t::AddChat(color_t color, const char* fmt, ...)
{
	char buf[512];
	
	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, 512, fmt, va);
	va_end(va);
	
	packet_t packet(id_chat_message);
	packet.write<id_t>(0);
	packet.write<time_t>(time(0));
	packet.write<color_t>(color);
	packet.write_string(buf);
	packet.write<bool>(false); // notify
	packet.write<bool>(true); // resend
	user_t::send(&packet);
}

void user_t::OnConnect()
{
	chat.send_history(this);

	if (m_status < 4)
		chat.sendf(1, m_id, m_color, "%s {f7f488}подключился", nick());

	_printf("[login] user: %s[%d], ip: %s", m_nick.c_str(), m_id, m_addr.c_str());
}

void user_t::OnDisconnect()
{
	if (m_status == 0)
		return;

	if (m_status < 4)
		chat.sendf(1, m_id, m_color, "%s {f7f488}отключился", nick());

	server.on_hide(this);

	_printf("[logout] user: %s[%d], ip: %s", m_nick.c_str(), m_id, m_addr.c_str());
}

void user_t::OnAuth(const std::string& key)
{
	packet_t packet(id_ht_clear_all);
	user_t::send(&packet);

	if (key_is_valid(key))
	{
		userdata_t ud;
		if (db.restore_user(key, &ud))
		{
			login(&ud);
			return;
		}
	}

	send_unreaded(0);
	
	server.on_count();
	server.send_online(this);
}

void user_t::OnPacket(packet_t* packet)
{
	server.m_crypt.decrypt(packet);

	int id = 0;
	packet->read<packet_id>((packet_id&)id);

	switch (id)
	{
	case id_user_auth:
		{
			std::string data;
			packet->read_string(data);
			OnAuth(data);
		}
		break;
	case id_user_input:
		{
			std::string data;
			packet->read_string(data);

			if (data[0] == '/')
				OnCommand(data);
			else
				OnChat(data);
		}
		break;
	case id_user_looks:
		{
			bool b;
			packet->read<bool>(b);
			bool u = m_watching != b && m_status;
			m_watching = b;
			if (u) server.on_nick(this);
			if (b && !m_motd && !m_status)
			{
				AddChat(0xFFFFFA66, "");
				AddChat(0xFFFFFA66, "\uf0eb {ffffff}Подсказка:");
				AddChat(0xFFFFFA66, "    Для входа используйте команды:");
				AddChat(0xFFFFFA66, "    \uf0a4    /auth {ffffff}<nick> <pass> {fffa66}- для авторизации");
				AddChat(0xFFFFFA66, "    \uf0a4    /register {ffffff}<nick> <pass> {fffa66}- для регистрации");
				AddChat(0xFFFFFA66, "");
				m_motd = true;
			}
		}
		break;
	}
}

void user_t::OnChat(const std::string& text)
{
	if (m_status == 0)
	{
		AddChat(0xFFDB0000, "[Ошибка] {ffffff}Требуется авторизация!");
		return;
	}

	if (text.size() > 256)
		return;

	std::string out = format_out(text, (m_status < 2));
	
	if (!out.empty())
		chat.pushf(1, m_id, m_color, "%s{ffffff}: %s", nick(), out.c_str());
}

void user_t::OnCommand(const std::string& text)
{
	if (text[0] != '/' || text.size() > 256)
		return;

	std::string cmd, arg;
	std::size_t pos = text.find(' ', 0);
	if (pos == std::string::npos)
	{
		cmd = text.substr(1);
	}
	else
	{
		cmd = text.substr(1, pos - 1);
		arg = text.substr(pos + 1);
	}

	if (cmd.size() == 0)
		return;

	cmds.run(this, cmd, arg);
}

void user_t::set_color(color_t color)
{
	m_color = (color | 0xFF000000);
	db.set_color(m_id, m_color);
	user_t::udn();
}

void user_t::set_nick(const std::string& nick)
{
	m_nick = nick;
	db.set_nick(m_id, m_nick);
	user_t::udn();
}

void user_t::set_prefix(const std::string& prefix)
{
	m_prefix = prefix;
	db.set_prefix(m_id, m_prefix);
	user_t::udn();
}

void user_t::set_status(int status)
{
	m_status = status;
	db.set_status(m_id, m_status);
	user_t::udn();
}
