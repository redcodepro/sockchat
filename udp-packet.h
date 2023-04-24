#include "main.h"

enum packet_id : int
{
	id_chat_init		= 0, // seed, name
	id_chat_auth		= 1, // ver, auth
	id_chat_event_		= 2, // deprecared
	id_chat_message		= 3,
	id_chat_unreaded	= 4,
	id_chat_erase		= 5,
	id_chat_erase_id	= 6,

	id_user_auth		= 10,
	id_user_input		= 11, // input
	id_user_watching	= 12,

	id_hudtext_init		= 20,
	id_hudtext_append	= 21,
	id_hudtext_clear	= 22,

	id_notify_play		= 30,
	id_notify_play_url	= 31,
	id_notify_set		= 32,
	id_notify_set_url	= 33,
};

typedef unsigned short len_t;
typedef unsigned int color_t;
typedef unsigned int id_t;

class ipacket_t
{
	friend class udpcrypt_t;
private:
	size_t len;
	size_t pos;
	uint8_t* data;
public:
	ipacket_t(uint8_t* _data, size_t _len) : data(_data), len(_len), pos(0) {}
	ipacket_t(ENetPacket* packet) : ipacket_t(packet->data, packet->dataLength) {}

	bool read(void* dst, size_t n);
	bool read_string(std::string& val);

	template <typename T>
	bool read(T& val) { return read(&val, sizeof(T)); }
};

class opacket_t
{
	friend class udpcrypt_t;
private:
	std::string data;
public:
	opacket_t(packet_id id) { write<packet_id>(id); }

	void write(const void* src, size_t n);
	void write_string(const std::string& val);

	template <typename T>
	void write(const T& val) { write(&val, sizeof(T)); }

	ENetPacket* to_enet(enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
};

static_assert(sizeof(id_t) == 4);
static_assert(sizeof(len_t) == 2);
static_assert(sizeof(color_t) == 4);
