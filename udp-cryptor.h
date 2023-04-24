#include "main.h"

class udpcrypt_t
{
private:
	unsigned int	m_seed = 0;
	int				m_table[256];
	int				m_table_rev[256];
public:
	void			init(unsigned int seed);
	void			reset() { m_seed = 0; }
	void			encrypt(opacket_t* packet);
	void			decrypt(ipacket_t* packet);
	unsigned int	seed() { return m_seed; }
};
