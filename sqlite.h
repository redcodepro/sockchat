#include "main.h"

#include <sqlite3.h>

class sqlite_t
{
private:
	sqlite3*		m_db			= nullptr;

	sqlite3_stmt*	m_add_user_1	= nullptr;
	sqlite3_stmt*	m_add_user_2	= nullptr;
	sqlite3_stmt*	m_find_user		= nullptr;
	sqlite3_stmt*	m_load_user		= nullptr;
	sqlite3_stmt*	m_auth_user		= nullptr;
	sqlite3_stmt*	m_restore_user	= nullptr;

	sqlite3_stmt*	m_set_nick		= nullptr;
	sqlite3_stmt*	m_set_color		= nullptr;
	sqlite3_stmt*	m_set_prefix	= nullptr;
	sqlite3_stmt*	m_set_status	= nullptr;
	
	sqlite3_stmt*	m_set_restore	= nullptr;
	sqlite3_stmt*	m_set_password	= nullptr;

	sqlite3_stmt*	m_find_blacklist= nullptr;
	sqlite3_stmt*	m_add_banip		= nullptr;
	sqlite3_stmt*	m_find_banip	= nullptr;

public:
	bool init(const char* filename);
	void free();

	int  add_user(const std::string& nick, const std::string& pass);
	int  find_user(const std::string& nick);
	bool load_user(int id, userdata_t* data);
	bool auth_user(int id, const std::string& pass, userdata_t* data);
	bool restore_user(const std::string& restore, userdata_t* data);

	void set_nick(int id, const std::string& nick);
	void set_color(int id, unsigned int color);
	void set_prefix(int id, const std::string& prefix);
	void set_status(int id, int status);

	void set_restore(int id, const std::string& restore);
	void set_password(int id, const std::string& password);

	bool find_blacklist(const std::string& nick);
	void add_banip(const std::string& ip);
	bool find_banip(const std::string& ip);
};

typedef sqlite_t database_t;
