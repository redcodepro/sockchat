#include "main.h"

enum packet_id : int
{
	id_chat_init		= 0, // seed, name
	id_chat_auth		= 1, // ver, auth
	id_chat_print		= 2, // color, text
	id_chat_message		= 3, // id, time, color, text, notify, resend
	id_chat_unreaded	= 4, // count
	id_chat_erase		= 5, // text
	id_chat_erase_id	= 6, // user id

	id_user_auth		= 10,
	id_user_input		= 11, // input
	id_user_looks		= 12, // state
	id_user_types		= 13,

	id_notify			= 50,

	id_ht_header_set	= 60, // text
	id_ht_footer_set	= 61, // text
	id_ht_entry_insert	= 62, // id, text
	id_ht_entry_remove	= 63, // id
	id_ht_clear_all		= 64,
};

static_assert(sizeof(packet_id) == sizeof(int));

class packet_t
{
	friend class udpcrypt_t;
private:
	ENetPacket*	packet;
	size_t		size;
	size_t		pos;

	void realloc(size_t size);
	static void free_callback(void* data);
public:
	packet_t(ENetPacket* _packet); // incoming
	packet_t(packet_id id, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE); // outgoing
	~packet_t();

	void write(const void* src, size_t n);
	void write_string(const std::string& val);

	template <typename T>
	void write(const T& val) { write(&val, sizeof(T)); }

	bool read(void* dst, size_t n);
	bool read_string(std::string& val);

	template <typename T>
	bool read(T& val) { return read(&val, sizeof(T)); }

	ENetPacket* get() { return packet; }
};
