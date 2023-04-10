#include "main.h"

#if (MYSQL_ENABLED == 1)

bool mysql_t::init(const char* address, const char* user, const char* password, const char* schema)
{
	try
	{
		sql::Driver* driver = get_driver_instance();
		m_connect = driver->connect(address, user, password);
		m_connect->setSchema(schema);
		m_stmt = m_connect->createStatement();
		return true;
	}
	catch (sql::SQLException& e)
	{
		_printf("[sql error] %s", e.what());
	}
	return false;
}

void mysql_t::free()
{
	_delete(m_stmt);
	_delete(m_connect);
}

bool mysql_t::query(const char* fmt, ...)
{
	char buf[2048];
	bool result = false;

	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, 2048, fmt, va);
	va_end(va);

	_printf("[sql query] %s", buf);

	try { result = m_stmt->execute(buf); }
	catch (sql::SQLException& e) { _printf("[sql error] %s", e.what()); }

	return result;
}

mysql_t::result_t mysql_t::query_result()
{
	return mysql_t::result_t(m_stmt->getResultSet());
}

int mysql_t::add_user(const std::string& nick, const std::string& pass)
{
	query("insert into `users` (`nick`) values ('%s');", nick.c_str());
	query("select last_insert_id() into @id;");
	query("insert into `auth` (`id`, `password`) values (@id, '%s');", pass.c_str());
	query("select @id;");

	result_t res = query_result();
	return res->next() ? res->getInt(1) : -1;
}

int mysql_t::find_user(const std::string& nick)
{
	query("select `id` from `users` where `nick`='%s';", nick.c_str());

	result_t res = query_result();
	return res->next() ? res->getInt(1) : -1;
}

bool mysql_t::load_user(int id, userdata_t* data)
{
	query("select * from `users` where `id`='%u';", id);

	result_t res = query_result();
	if (res->next() == false)
		return false;
	
	data->m_id = res->getInt(1);
	data->m_nick = res->getString(2);
	data->m_color = res->getInt(3);
	data->m_prefix = res->getString(4);
	data->m_status = res->getInt(5);

	return true;
}

bool mysql_t::auth_user(int id, const std::string& pass, userdata_t* data)
{
	query("select 1 from `auth` where `id`='%d' and `password`='%s';", id, pass.c_str());

	result_t res = query_result();
	return res->next() ? load_user(id, data) : false;
}

bool mysql_t::restore_user(const std::string& restore, userdata_t* data)
{
	query("select `id` from `auth` where `restore`='%s';", restore.c_str());

	result_t res = query_result();
	return res->next() ? load_user(res->getInt(1), data) : false;
}

void mysql_t::set_restore(int id, const std::string& restore)
{
	query("update `auth` set `restore`='%s' where `id`='%d';", restore.c_str(), id);
}

void mysql_t::set_password(int id, const std::string& pass)
{
	query("update `auth` set `password`='%s' where `id`='%d';", pass.c_str(), id);
}

bool mysql_t::find_blacklist(const std::string& nick)
{
	query("select 1 from `blacklist` where `nick`='%s';", nick.c_str());

	result_t res = query_result();
	return res->next();
}

void mysql_t::set_nick(int id, const std::string& nick)
{
	query("update `users` set `nick`='%s' where `id`='%d';", nick.c_str(), id);
}

void mysql_t::set_color(int id, unsigned int color)
{
	query("update `users` set `color`='%d' where `id`='%d';", color, id);
}

void mysql_t::set_prefix(int id, const std::string& prefix)
{
	query("update `users` set `prefix`='%s' where `id`='%d';", prefix.c_str(), id);
}

void mysql_t::set_status(int id, int status)
{
	query("update `users` set `status`='%d' where `id`='%d';", status, id);
}

void mysql_t::add_banip(const std::string& ip)
{
	query("insert into `banip` (`ip`) values ('%s');", ip.c_str());
}

void mysql_t::load_banip()
{
	query("select `ip` from `banip`;");

	result_t res = query_result();
	while (res->next())
	{
		std::string str = res->getString(1);
		in_addr_t addr = inet_addr(str.c_str());
		if (addr != INADDR_NONE)
			server.add_banip(addr);
	}
}

sql::SQLString mysql_t::escape_string(const sql::SQLString& in)
{
	return reinterpret_cast<sql::mysql::MySQL_Connection*>(m_connect)->escapeString(in);
}

#endif
