#include "main.h" // ÙŦF-8

void cmd_register(user_t* user, const char* nick, const char* pass)
{
	if (!nick_is_valid(nick, true))
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Ник не подходит!");
		return;
	}

	if (strlen(pass) < 5)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Пароль слишком короткий!");
		return;
	}

	if (db.find_user(nick) != -1 || db.find_blacklist(nick))
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Ник занят!");
		return;
	}

	int id = db.add_user(nick, md5(pass));
	if (id != -1)
	{
		userdata_t ud;
		if (db.load_user(id, &ud))
			user->login(&ud);
	}
}

void cmd_auth(user_t* user, const char* nick, const char* pass)
{
	if (!nick_is_valid(nick, false) || strlen(pass) < 5)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Что-то введено не верно!");
		return;
	}

	int id = db.find_user(nick);
	if (id == -1)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Аккаунт не существует!");
		return;
	}

	userdata_t ud;
	if (!db.auth_user(id, md5(pass), &ud))
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Пароль неправильный!");
		return;
	}

	user->login(&ud);
}

void cmd_setpass(user_t* user, const char* pass)
{
	if (strlen(pass) < 5)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {FFFFFF}Пароль слишком короткий!");
		return;
	}

	db.set_password(user->m_id, md5(pass));
	user->AddChat(0xFF9CFF9F, "Пароль изменён на {FFFFFF}\"%s\"{9cff9f}. Используйте {FFFFFF}F8 {9cff9f}что-бы сохранить.", pass);
}

void cmd_ping(user_t* user)
{
	user->AddChat(0xFFF7F488, "Pong!");
}

void cmd_time(user_t* user)
{
	time_t time_ = time(0);
	struct tm tm_;
	localtime_r(&time_, &tm_);
	user->AddChat(0xFFF7F488, "Время: {ffffff}%02d:%02d:%02d", tm_.tm_hour, tm_.tm_min, tm_.tm_sec);
}

void cmd_me(user_t* user, const char* text)
{
	chat.pushf(1, user->m_id, user->m_color, "%s {ff99ff}%s", user->nick(), format_out(text).c_str());
}

void cmd_do(user_t* user, const char* text)
{
	chat.pushf(1, user->m_id, 0xFF4682B4, "%s - | %s", format_out(text).c_str(), user->nick_c());
}

void cmd_exit(user_t* user)
{
	user->logout();
}

void cmd_clear(user_t* user)
{
	chat.clear();
	server.AddEventGlobal("erase");
	chat.pushf(0, 0, 0xFFF02E2E, "Чат очищен администратором.");
}

void cmd_update(user_t* user)
{
	user->send_event("erase");
	chat.send_history(user);
}

void cmd_erase(user_t* user, const char* text)
{
	chat.clear(text);
	server.AddEventGlobal(std::string("erase=") + text);
}

void cmd_clearuser(user_t* user, int id)
{
	int c = chat.clear_id(id);
	server.AddEventGlobal(std::string("erase-id=") + std::to_string(id));
	user->AddChat(0xFF00FF00, "[Информация] {ffffff}Удалено %d сообщений.", c);
}

void cmd_msg(user_t* user, int id, const char* text)
{
	if (id == user->m_id)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Абонент не абонент!");
		return;
	}

	user_t* usr = server.find_user(id);
	if (usr == nullptr)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Пользователь оффлайн!");
		return;
	}

	server.SendPM(user, usr, text);
}

void cmd_msg_re(user_t* user, const char* text)
{
	if (user->m_xid == -1)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Некому отвечать");
		return;
	}

	user_t* usr = server.find_user(user->m_xid);
	if (usr == nullptr)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Пользователь оффлайн!");
		return;
	}

	server.SendPM(user, usr, text);
}

user_t* find_target(user_t* user, int id)
{
	user_t* target = server.find_user(id);
	if (target == nullptr)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Пользователь не найден!");
		return nullptr;
	}

	if (target->m_id != user->m_id && target->m_status >= user->m_status)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Нет доступа!");
		return nullptr;
	}

	return target;
}

void cmd_kick(user_t* user, int id, const char* reason)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	server.KickUser(usr);
	chat.pushf(1, user->m_id, 0xFFFF6347, "Модератор %s {ff6347}кикнул %s{ff6347}. Причина: {ffffff}%s", user->nick_c(), usr->nick_c(), reason);
}

void cmd_skick(user_t* user, int id)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	server.KickUser(usr);
	user->AddChat(0xFFDB0000, ">> %s кикнут.", usr->nick());
}

void cmd_ban(user_t* user, int id, const char* reason)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	server.MakeBan(usr);
	chat.pushf(1, user->m_id, 0xFFFF6347, "Модератор %s {ff6347}забанил %s{ff6347}. Причина: {ffffff}%s", user->nick_c(), usr->nick_c(), reason);
}

void cmd_banip(user_t* user, int id, const char* reason)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	server.MakeBanIP(usr);
	chat.pushf(1, user->m_id, 0xFFCC1F00, "Модератор %s {ff6347}забанил %s{cc1f00}. Причина: {ffffff}%s", user->nick_c(), usr->nick_c(), reason);
}

void cmd_destroy(user_t* user, int id)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	server.MakeBan(usr);
	cmd_clearuser(user, id);
	user->AddChat(0xFFDB0000, ">> %s уничтожен.", usr->nick());
}

void cmd_setcolor(user_t* user, int id, color_t color)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	usr->set_color(color);
	user->AddChat(0xFFDB0000, ">> Установлен цвет для %s", usr->nick_c());
}

void cmd_setnick(user_t* user, int id, const char* nick)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

//	if (!nick_is_valid(nick, false))
//	{
//		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Ник не подходит!");
//		return;
//	}

	usr->set_nick(nick);
	user->AddChat(0xFFDB0000, ">> Установлен ник для %s", usr->nick_c());
}

void cmd_setprefix(user_t* user, int id, const char* prefix)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	if (!strcmp(prefix, "-"))
	{
		usr->set_prefix("");
		user->AddChat(0xFFDB0000, ">> Сброшен префикс для %s", usr->nick_c());
	}
	else
	{
		usr->set_prefix(prefix);
		user->AddChat(0xFFDB0000, ">> Установлен префикс для %s", usr->nick_c());
	}
}

void cmd_setstatus(user_t* user, int id, int status)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	if (status < 1 || status > 4)
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Невозможно установить статус!");
		return;
	}

	usr->set_status(status);
	user->AddChat(0xFFDB0000, ">> Установлен статус {ffffff}%d {db0000}для %s", status, usr->nick_c());
}

void cmd_rainbow(user_t* user, int id)
{
	user_t* usr = find_target(user, id);
	if (usr == nullptr)
		return;

	usr->m_rainbow ^= 1;
}

void cmd_hideme(user_t* user)
{
	user->m_hideme ^= 1;
	user->AddChat(0xFFDB0000, ">> Неведимка %s.", user->m_hideme ? "включена" : "выключена");
	user->udn();
}

void cmd_chat_vip(user_t* user, const char* text)
{
	chat.pushf(2, user->m_id, 0xFF8B61FF, "[VIP] %s{ffffff}: %s", user->nick_c(), text);
}

void cmd_chat_adm(user_t* user, const char* text)
{
	chat.pushf(3, user->m_id, 0xFFA6E300, "[A] %s{a6e300}: %s", user->nick_c(), text);
}

void cmd_chat_all(user_t* user, const char* text)
{
	chat.pushf(0, user->m_id, 0xFFFFE500, "[G] %s{ffe500}: %s", user->nick_c(), text);
}

void cmd_notify(user_t* user)
{
	user->m_notify ^= 1;
	user->AddChat(0xFF00FF00, "[Информация] {ffffff}Звук входящих сообщений %s", user->m_notify ? "{00ff00}включен" : "{ff0000}выключен");
}

void cmd_notify_me(user_t* user)
{
	user->send_notify();
}

void cmd_setnotify(user_t* user, const char* name)
{
	std::string url = find_audio_url(name);
	if (url.empty())
	{
		if (!strcmp(name, "-"))
		{
			server.NotifySet("");
			user->AddChat(0xFF00FF00, "[Информация] {ffffff}Установлен стандартный звук уведомлений.");
			return;
		}
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Файл не существует!");
		return;
	}

	server.NotifySet(url);
	user->AddChat(0xFF00FF00, "[Информация] {ffffff}Установлен звук уведомлений: {0077e5}\"%s\"", url.c_str());
}

void cmd_play_all(user_t* user, const char* name)
{
	std::string url = find_audio_url(name);
	if (url.empty())
	{
		user->AddChat(0xFFDB0000, "[Ошибка] {ffffff}Файл не существует!");
		return;
	}

	server.NotifyPlay(url);
	user->AddChat(0xFF00FF00, "[Информация] {ffffff}Проигрываю: {0077e5}\"%s\"", url.c_str());
}

void cmd_tts_all(user_t* user, const char* text)
{
	std::string url = "https://tts.voicetech.yandex.net/generate"
		"?key=6372dda5-9674-4413-85ff-e9d0eb2f99a7"
		"&format=mp3"
		"&emotion=neutral"
		"&speaker=voicesearch"
		"&text=";
	url += urlencode(text);
	server.NotifyPlay(url);
	user->AddChat(0xFF00FF00, "[Информация] {ffffff}Проигрываю: {ffc800}YTTS: \"%s\"", text);
}

void init_commands()
{
	cmds.add({ "reg", "register" },		new cmd_t{ 0, (void*)cmd_register,	"ss",	"<nick> <pass>"	});
	cmds.add({ "auth", "login" },		new cmd_t{ 0, (void*)cmd_auth,		"ss",	"<nick> <pass>"	});
	cmds.add({ "setpass", "passwd" },	new cmd_t{ 1, (void*)cmd_setpass,	"s",	"<pass>"		});
//	cmds.add({ "ping" },				new cmd_t{ 1, (void*)cmd_ping,		"",		""				});
//	cmds.add({ "time" },				new cmd_t{ 1, (void*)cmd_time,		"",		""				});
	cmds.add({ "msg", "pm", "sms" },	new cmd_t{ 1, (void*)cmd_msg,		"d*",	"<id> <text>"	});
	cmds.add({ "re", "r" },				new cmd_t{ 1, (void*)cmd_msg_re,	"*",	"<text>"		});
	cmds.add({ "me" },					new cmd_t{ 1, (void*)cmd_me,		"*",	"<text>"		});
	cmds.add({ "do" },					new cmd_t{ 1, (void*)cmd_do,		"*",	"<text>"		});
	cmds.add({ "exit", "quit" },		new cmd_t{ 2, (void*)cmd_exit,		"",		""				});
	cmds.add({ "update" },				new cmd_t{ 2, (void*)cmd_update,	"",		""				});
//	cmds.add({ "vr", "vip" },			new cmd_t{ 2, (void*)cmd_chat_vip,	"*",	"<text>"		});
	cmds.add({ "a" },					new cmd_t{ 3, (void*)cmd_chat_adm,	"*",	"<text>"		});
	cmds.add({ "g", "ao" },				new cmd_t{ 3, (void*)cmd_chat_all,	"*",	"<text>"		});
	cmds.add({ "erase" },				new cmd_t{ 3, (void*)cmd_erase,		"*",	"<text>"		});
	cmds.add({ "kick" },				new cmd_t{ 3, (void*)cmd_kick,		"d*",	"<id> <reason>" });
	cmds.add({ "skick" },				new cmd_t{ 4, (void*)cmd_skick,		"d",	"<id>"			});
	cmds.add({ "ban" },					new cmd_t{ 3, (void*)cmd_ban,		"d*",	"<id> <reason>" });
	cmds.add({ "banip" },				new cmd_t{ 4, (void*)cmd_banip,		"d*",	"<id> <reason>"	});
	cmds.add({ "cc", "clear_chat" },	new cmd_t{ 4, (void*)cmd_clear,		"",		""				});
	cmds.add({ "cu", "clear_user" },	new cmd_t{ 3, (void*)cmd_clearuser,	"d",	"<id>"			});
	cmds.add({ "destroy" },				new cmd_t{ 4, (void*)cmd_destroy,	"d",	"<id>"			});
	cmds.add({ "setcolor" },			new cmd_t{ 4, (void*)cmd_setcolor,	"dx",	"<id> <color>"	});
	cmds.add({ "setprefix" },			new cmd_t{ 4, (void*)cmd_setprefix,	"d*",	"<id> <prefix>"	});
	cmds.add({ "setnick" },				new cmd_t{ 5, (void*)cmd_setnick,	"ds",	"<id> <nick>"	});
	cmds.add({ "setstatus" },			new cmd_t{ 5, (void*)cmd_setstatus,	"dd",	"<id> <status>"	});
	cmds.add({ "rainbow", "makegay" },	new cmd_t{ 5, (void*)cmd_rainbow,	"d",	"<id>"			});
	cmds.add({ "hideme" },				new cmd_t{ 4, (void*)cmd_hideme,	"",		""				});
	cmds.add({ "notify" },				new cmd_t{ 1, (void*)cmd_notify,	"",		""				});
//	cmds.add({ "notify_test" },			new cmd_t{ 1, (void*)cmd_notify_me,	"",		""				});
//	cmds.add({ "notify_set" },			new cmd_t{ 5, (void*)cmd_setnotify,	"*",	"<name>"		});
	cmds.add({ "play" },				new cmd_t{ 5, (void*)cmd_play_all,	"*",	"<name>"		});
	cmds.add({ "tts", "say" },			new cmd_t{ 5, (void*)cmd_tts_all,	"*",	"<text>"		});

	_printf("[info] init_commands(): %d commands loaded.", cmds.get_count());
}
