#pragma once

enum packet_id
{
	id_conn_request		= 0, // ver, auth!1.0
	id_ping				= 1,
	id_pong				= 2,
	id_conn_init		= 4, // cookie, timeout, enc_seed
	id_conn_rejected	= 5,
	id_conn_closed		= 6,
	id_conn_banned		= 7,
	id_user_auth		= 8, // authkey

	id_hudtext_init		= 19,
	id_hudtext_append	= 20,
	id_hudtext_clear	= 21,
	id_set_unreaded		= 22, // count

	id_notify_play		= 25,
	id_notify_set_url	= 26,
	id_notify_play_url	= 27,

	id_auth				= 'a',
	id_event			= 'e',
	id_message			= '<',
	id_user_chat		= '>',
	id_user_command		= '/',
	id_watching			= 'w',
};

typedef unsigned short len_t;
typedef unsigned int color_t;
typedef unsigned int id_t;

class packet_header_t
{
public:
	short cookie = 0;
	len_t id = 0;
	len_t len = 0;
	len_t pos = 0;
};

#define packet_max_size		1472
#define packet_data_size	(packet_max_size - sizeof(packet_header_t))

class packet_t : public packet_header_t
{
public:
	char data[packet_data_size];

	packet_t() {}
	packet_t(packet_id _id) { id = _id; }

	bool Write(const void* src, len_t n);
	bool Read(void* dst, len_t n);

	template <typename T>
	bool Write(T val) { return Write(&val, sizeof(T)); }
	template <typename T>
	bool Read(T& val) { return Read(&val, sizeof(T)); }

	bool WriteString(const std::string& in);
	bool ReadString(std::string& val);
	std::string ReadString() { std::string out; ReadString(out); return out; }
};

static_assert(sizeof(id_t) == 4);
static_assert(sizeof(len_t) == 2);
static_assert(sizeof(color_t) == 4);
static_assert(sizeof(packet_t) == packet_max_size);
