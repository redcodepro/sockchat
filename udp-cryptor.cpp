#include "main.h"

void create_table(enet_uint32 seed, enet_uint8* out, enet_uint8* rev = nullptr)
{
	std::mt19937 genrand(seed);
	for (int i = 0; i < 256; i++) {
		while (true) {
			bool brk = true;
			enet_uint8 chr = (enet_uint8)(genrand() & 0xFF);

			for (int j = 0; j < i; j++) {
				if (out[j] == chr) {
					brk = false;
					break;
				}
			}

			if (brk) {
				out[i] = chr;
				if (rev) rev[chr] = static_cast<enet_uint8>(i);
				break;
			}
		}
	}
}

void udpcrypt_t::init(enet_uint32 seed)
{
	m_seed = seed;
	create_table(m_seed, m_table, m_table_rev);
}

void udpcrypt_t::encrypt(packet_t* packet)
{
	if (m_seed == 0)
		return;

	size_t len = packet->packet->dataLength;
	enet_uint8* data = packet->packet->data;

	for (; len; len--) {
		auto& c = *data++;
		c = m_table[c];
	}
}

void udpcrypt_t::decrypt(packet_t* packet)
{
	if (m_seed == 0)
		return;

	size_t len = packet->packet->dataLength;
	enet_uint8* data = packet->packet->data;

	for (; len; len--) {
		auto& c = *data++;
		c = m_table_rev[c];
	}
}
