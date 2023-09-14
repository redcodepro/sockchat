#include "main.h"

enum packet_id : int
{
	id_chat_init		= 0, // seed, name
	id_chat_auth		= 1, // ver, auth
	id_chat_event_		= 2, // deprecated
	id_chat_message		= 3,
	id_chat_unreaded	= 4,
	id_chat_erase		= 5,
	id_chat_erase_id	= 6,

	id_user_auth		= 10,
	id_user_input		= 11, // input
	id_user_looks		= 12,
	id_user_types		= 13,

	id_hudtext_init		= 20,
	id_hudtext_append	= 21,
	id_hudtext_clear	= 22,

	_id_notify_play_url	= 31, // remove in 2.1 (server)

	id_ht_header_set	= 40, // text
	id_ht_footer_set	= 41, // text
	id_ht_entry_insert	= 42, // id, text
	id_ht_entry_remove	= 43, // id
	id_ht_clear_all		= 44,

	id_notify			= 50,
	id_notify_set		= 51, // audio packet
	id_notify_set_url	= 52, // url
	id_audio_play		= 53, // audio packet
	id_audio_play_url	= 54, // url
	id_audio_stop		= 55,
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
