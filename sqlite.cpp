#include "main.h"

#if (MYSQL_ENABLED == 0)

#define errlog(code) \
	if (code != SQLITE_OK) \
	{ \
		_printf("[sql error] %s", sqlite3_errmsg(m_db)); \
		return false; \
	}

#define errexec(db, query) \
		errlog(sqlite3_exec(db, query, 0, 0, 0))

#define errprep(db, query, stmt) \
	errlog(sqlite3_prepare_v2(db, query, -1, stmt, NULL))

#define errstep(stmt, res, retval) \
	if (sqlite3_step(stmt) != res) \
	{ \
		_printf("[sql error] %s", sqlite3_errmsg(m_db)); \
		return retval; \
	}

bool sqlite_t::init(const char* filename)
{
	if (sqlite3_open(filename, &m_db) != SQLITE_OK)
	{
		if (m_db)
		{
			_printf("[sql error] %s", sqlite3_errmsg(m_db));
			sqlite3_close(m_db);
		}
		return false;
	}

	errexec(m_db,	"create table if not exists `banip` ( \
						`ip` text not null \
					);");

	errexec(m_db,	"create table if not exists `blacklist` ( \
						`nick` text not null \
					);");

	errexec(m_db,	"create table if not exists `auth` ( \
						`id` integer not null primary key, \
						`password` text, \
						`restore` text \
					);");
	errexec(m_db,	"create table if not exists `users` ( \
						`id` integer not null primary key autoincrement, \
						`nick` text not null, \
						`color` integer not null default -1, \
						`prefix` text not null default '', \
						`status` integer default 1 \
					);");
	
	errexec(m_db, "insert or ignore into `users` (`id`, `nick`, `color`, `prefix`, `status`) values (1, 'admin', -65536, '', 5);");
	errexec(m_db, "insert or ignore into `auth` (`id`, `password`, `restore`) values (1, '4297f44b13955235245b2497399d7a93', '');");

	errprep(m_db, "insert into `users` (`nick`) values (?);",				&m_add_user_1);
	errprep(m_db, "insert into `auth` (`id`, `password`) values (?, ?);",	&m_add_user_2);
	errprep(m_db, "select `id` from `users` where `nick`=?;",				&m_find_user);
	errprep(m_db, "select * from `users` where `id`=?;",					&m_load_user);
	errprep(m_db, "select 1 from `auth` where `id`=? and `password`=?;",	&m_auth_user);
	errprep(m_db, "select `id` from `auth` where `restore`=?;",				&m_restore_user);
	errprep(m_db, "update `users` set `nick`=? where `id`=?;",				&m_set_nick);
	errprep(m_db, "update `users` set `color`=? where `id`=?;",				&m_set_color);
	errprep(m_db, "update `users` set `prefix`=? where `id`=?;",			&m_set_prefix);
	errprep(m_db, "update `users` set `status`=? where `id`=?;",			&m_set_status);
	errprep(m_db, "update `auth` set `restore`=? where `id`=?;",			&m_set_restore);
	errprep(m_db, "update `auth` set `password`=? where `id`=?;",			&m_set_password);
	errprep(m_db, "select 1 from `blacklist` where `nick`=?;",				&m_find_blacklist);
	errprep(m_db, "insert into `banip` (`ip`) values (?);",					&m_add_banip);
	errprep(m_db, "select 1 from `banip` where `ip`=?;",					&m_find_banip);

	return true;
}

void sqlite_t::free()
{
	if (m_db != nullptr)
	{
		sqlite3_close(m_db);
		m_db = nullptr;
	}
}

int sqlite_t::add_user(const std::string& nick, const std::string& pass)
{
	sqlite3_reset(m_add_user_1);
	sqlite3_clear_bindings(m_add_user_1);
	sqlite3_bind_text(m_add_user_1, 1, nick.c_str(), -1, SQLITE_TRANSIENT);
	errstep(m_add_user_1, SQLITE_DONE, -1);

	int id = static_cast<int>(sqlite3_last_insert_rowid(m_db));

	sqlite3_reset(m_add_user_2);
	sqlite3_clear_bindings(m_add_user_2);
	sqlite3_bind_int(m_add_user_2, 1, id);
	sqlite3_bind_text(m_add_user_2, 2, pass.c_str(), -1, SQLITE_TRANSIENT);
	errstep(m_add_user_2, SQLITE_DONE, -1);

	return id;
}

int sqlite_t::find_user(const std::string& nick)
{
	sqlite3_reset(m_find_user);
	sqlite3_clear_bindings(m_find_user);
	sqlite3_bind_text(m_find_user, 1, nick.c_str(), -1, SQLITE_TRANSIENT);
	return (sqlite3_step(m_find_user) == SQLITE_ROW) ? sqlite3_column_int(m_find_user, 0) : -1;
}

bool sqlite_t::load_user(int id, userdata_t* data)
{
	sqlite3_reset(m_load_user);
	sqlite3_clear_bindings(m_load_user);
	sqlite3_bind_int(m_load_user, 1, id);
	errstep(m_load_user, SQLITE_ROW, false);

	data->m_id = sqlite3_column_int(m_load_user, 0);
	data->m_nick = (char*)sqlite3_column_text(m_load_user, 1);;
	data->m_color = sqlite3_column_int(m_load_user, 2);
	data->m_prefix = (char*)sqlite3_column_text(m_load_user, 3);
	data->m_status = sqlite3_column_int(m_load_user, 4);

	return true;
}

bool sqlite_t::auth_user(int id, const std::string& pass, userdata_t* data)
{
	sqlite3_reset(m_auth_user);
	sqlite3_clear_bindings(m_auth_user);
	sqlite3_bind_int(m_auth_user, 1, id);
	sqlite3_bind_text(m_auth_user, 2, pass.c_str(), -1, SQLITE_TRANSIENT);

	return (sqlite3_step(m_auth_user) == SQLITE_ROW) ? load_user(id, data) : false;
}

bool sqlite_t::restore_user(const std::string& restore, userdata_t* data)
{
	sqlite3_reset(m_restore_user);
	sqlite3_clear_bindings(m_restore_user);
	sqlite3_bind_text(m_restore_user, 1, restore.c_str(), -1, SQLITE_TRANSIENT);
	
	return (sqlite3_step(m_restore_user) == SQLITE_ROW) ? load_user(sqlite3_column_int(m_restore_user, 0), data) : false;
}

void sqlite_t::set_restore(int id, const std::string& restore)
{
	sqlite3_reset(m_set_restore);
	sqlite3_clear_bindings(m_set_restore);
	sqlite3_bind_text(m_set_restore, 1, restore.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(m_set_restore, 2, id);
	errstep(m_set_restore, SQLITE_DONE, );
}

void sqlite_t::set_password(int id, const std::string& pass)
{
	sqlite3_reset(m_set_password);
	sqlite3_clear_bindings(m_set_password);
	sqlite3_bind_text(m_set_password, 1, pass.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(m_set_password, 2, id);
	errstep(m_set_password, SQLITE_DONE, );
}

bool sqlite_t::find_blacklist(const std::string& nick)
{
	sqlite3_reset(m_find_blacklist);
	sqlite3_clear_bindings(m_find_blacklist);
	sqlite3_bind_text(m_find_blacklist, 1, nick.c_str(), -1, SQLITE_TRANSIENT);
	return (sqlite3_step(m_find_blacklist) == SQLITE_ROW);
}

void sqlite_t::set_nick(int id, const std::string& nick)
{
	sqlite3_reset(m_set_nick);
	sqlite3_clear_bindings(m_set_nick);
	sqlite3_bind_text(m_set_nick, 1, nick.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(m_set_nick, 2, id);
	errstep(m_set_nick, SQLITE_DONE, );
}

void sqlite_t::set_color(int id, unsigned int color)
{
	sqlite3_reset(m_set_color);
	sqlite3_clear_bindings(m_set_color);
	sqlite3_bind_int(m_set_color, 1, (int)color);
	sqlite3_bind_int(m_set_color, 2, id);
	errstep(m_set_color, SQLITE_DONE, );
}

void sqlite_t::set_prefix(int id, const std::string& prefix)
{
	sqlite3_reset(m_set_prefix);
	sqlite3_clear_bindings(m_set_prefix);
	sqlite3_bind_text(m_set_prefix, 1, prefix.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(m_set_prefix, 2, id);
	errstep(m_set_prefix, SQLITE_DONE, );
}

void sqlite_t::set_status(int id, int status)
{
	sqlite3_reset(m_set_status);
	sqlite3_clear_bindings(m_set_status);
	sqlite3_bind_int(m_set_status, 1, status);
	sqlite3_bind_int(m_set_status, 2, id);
	errstep(m_set_status, SQLITE_DONE, );
}

void sqlite_t::add_banip(const std::string& ip)
{
	sqlite3_reset(m_add_banip);
	sqlite3_clear_bindings(m_add_banip);
	sqlite3_bind_text(m_add_banip, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
	errstep(m_add_banip, SQLITE_DONE, );
}

bool sqlite_t::find_banip(const std::string& ip)
{
	sqlite3_reset(m_find_banip);
	sqlite3_clear_bindings(m_find_banip);
	sqlite3_bind_text(m_find_banip, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
	return (sqlite3_step(m_find_banip) == SQLITE_ROW);
}

#endif