#include "main.h"

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>

struct mysql_user_t
{
	int			m_id;
	std::string m_nick;
	int			m_color;
	std::string m_prefix;
	int         m_status;
};

class mysql_t
{
private:
	typedef std::shared_ptr<sql::ResultSet> result_t;

	bool		query(const char* fmt, ...);
	result_t	query_result();

	sql::Connection*	m_connect	= nullptr;
	sql::Statement*		m_stmt		= nullptr;
public:
	bool init(const char* address, const char* user, const char* password, const char* schema);
	void free();

	int  add_user(const std::string& nick, const std::string& pass);
	int  find_user(const std::string& nick);
	bool load_user(int id, mysql_user_t* data);
	bool auth_user(int id, const std::string& pass, mysql_user_t* data);
	bool restore_user(const std::string& restore, mysql_user_t* data);

	void set_nick(int id, const std::string& nick);
	void set_color(int id, unsigned int color);
	void set_prefix(int id, const std::string& prefix);
	void set_status(int id, int status);

	void set_restore(int id, const std::string& restore);
	void set_password(int id, const std::string& password);

	bool find_blacklist(const std::string& nick);
	void add_banip(const std::string& ip);
	void load_banip();

	sql::SQLString escape_string(const sql::SQLString& in);
};
