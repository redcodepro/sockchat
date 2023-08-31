#include "main.h"

class udpcrypt_t
{
private:
	enet_uint32 m_seed = 0;
	enet_uint8	m_table[256];
	enet_uint8	m_table_rev[256];
public:
	void		init(enet_uint32 seed);
	void		reset() { m_seed = 0; }
	void		encrypt(packet_t* packet);
	void		decrypt(packet_t* packet);
	enet_uint32	seed() { return m_seed; }
};
