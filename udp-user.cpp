﻿#include "main.h" // ÙŦF-8

void user_t::udn()
{
	char buf[512];
	sprintf(buf, "{%06x}%s%s[%d]", (m_color & 0xFFFFFF), m_prefix.c_str(), m_nick.c_str(), m_id);
	m_displayname = buf + 8;
	m_displayname_c = buf;
	server.on_udn();
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
}

void user_t::logout()
{
	OnDisconnect();

	m_status = 0;
	m_prefix = "";
	m_color = -1;
	m_nick = "";
	m_id = -1;

	user_t::send_auth("");
}

void user_t::send(opacket_t* packet)
{
	server.m_crypt.encrypt(packet);
	enet_peer_send(m_peer, 0, packet->to_enet());
}

void user_t::send_auth(const std::string& auth)
{
	opacket_t packet(id_chat_auth);
	packet.write_string(auth);
	user_t::send(&packet);
}

void user_t::send_hudtext(const std::string& text)
{
	opacket_t packet(id_hudtext_init);
	packet.write_string(text);
	user_t::send(&packet);
}

void user_t::send_unreaded(int count)
{
	opacket_t packet(id_chat_unreaded);
	packet.write<int>(count);
	user_t::send(&packet);
}

void user_t::send_notify()
{
	opacket_t packet(id_notify_play);
	user_t::send(&packet);
}

void user_t::send_notify_set(const std::string& url)
{
	opacket_t packet(id_notify_set_url);
	packet.write_string(url);
	user_t::send(&packet);
}

void user_t::send_notify_play(const std::string& url)
{
	opacket_t packet(id_notify_play_url);
	packet.write_string(url);
	user_t::send(&packet);
}

void user_t::send_erase(const std::string& text)
{
	opacket_t packet(id_chat_erase);
	packet.write_string(text);
	user_t::send(&packet);
}

void user_t::send_erase(id_t id)
{
	opacket_t packet(id_chat_erase);
	packet.write<id_t>(id);
	user_t::send(&packet);
}

void user_t::AddChat(color_t color, const char* fmt, ...)
{
	char buf[512];
	
	va_list va;
	va_start(va, fmt);
	int count = vsnprintf(buf, 512, fmt, va);
	va_end(va);
	
	if (count < 0) return;
	if (count > 512) count = 512;
	
	opacket_t packet(id_chat_message);
	packet.write<id_t>(0);
	packet.write<time_t>(time(0));
	packet.write<color_t>(color);
	packet.write_string(buf);
	packet.write<bool>(false); // notify
	packet.write<bool>(false); // resend
	user_t::send(&packet);
}

void user_t::OnConnect()
{
	chat.send_history(this);

	if (m_status < 4)
		chat.sendf(1, m_id, m_color, "%s {f7f488}подключился", nick());

	_printf("[login] user: %s[%d], ip: %s", m_nick.c_str(), m_id, addr(&m_peer->address));
}

void user_t::OnDisconnect()
{
	if (m_status == 0)
		return;

	if (m_status < 4)
		chat.sendf(1, m_id, m_color, "%s {f7f488}отключился", nick());

	server.on_udn();

	_printf("[logout] user: %s[%d], ip: %s", m_nick.c_str(), m_id, addr(&m_peer->address));
}

void user_t::OnAuth(const std::string& key)
{
	//send_erase("");

	if (key_is_valid(key))
	{
		userdata_t ud;
		if (db.restore_user(key, &ud))
		{
			login(&ud);
			return;
		}
	}

	AddChat(0xFFFFFA66, "");
	AddChat(0xFFFFFA66, "\uf0eb {ffffff}Подсказка:");
	AddChat(0xFFFFFA66, "    Для входа используйте команды:");
	AddChat(0xFFFFFA66, "    \uf0a4    /auth {ffffff}<nick> <pass> {fffa66}- для авторизации");
	AddChat(0xFFFFFA66, "    \uf0a4    /register {ffffff}<nick> <pass> {fffa66}- для регистрации");
	AddChat(0xFFFFFA66, "");
	AddChat(0xFF00FF00, "[Информация] {ffffff}Аккаунты перенесены с WebChat-a.");
	send_unreaded(1);
}

void user_t::OnPacket(ipacket_t* packet)
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
	case id_user_watching:
		{
			bool b;
			packet->read<bool>(b);
			if (m_watching != b && m_status != 0)
				server.on_udn();
			m_watching = b;
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
}
