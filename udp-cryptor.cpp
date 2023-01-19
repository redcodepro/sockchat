#include "main.h"

namespace genrand { // mt19937
	constexpr int N = 624;
	constexpr int M = 397;
	constexpr unsigned long MATRIX_A = 0x9908b0dfUL;
	constexpr unsigned long UPPER_MASK = 0x80000000UL;
	constexpr unsigned long LOWER_MASK = 0x7fffffffUL;

	static unsigned long mt[N];
	static int mti = N + 1;

	void init(unsigned long s)
	{
		mt[0] = s & 0xffffffffUL;
		for (mti = 1; mti < N; mti++) {
			mt[mti] = (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
			mt[mti] &= 0xffffffffUL;
		}
	}

	unsigned long int32()
	{
		unsigned long y;
		static unsigned long mag01[2] = { 0x0UL, MATRIX_A };

		if (mti >= N) {
			int kk;

			if (mti == N + 1)
				init(5489UL);

			for (kk = 0; kk < N - M; kk++) {
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			for (; kk < N - 1; kk++) {
				y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
				mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
			mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

			mti = 0;
		}

		y = mt[mti++];

		y ^= (y >> 11);
		y ^= (y << 7) & 0x9d2c5680UL;
		y ^= (y << 15) & 0xefc60000UL;
		y ^= (y >> 18);

		return y;
	}
}

void create_table(unsigned int seed, int* out, int* rev = nullptr)
{
	genrand::init(seed);
	for (int i = 0; i < 256; i++) {
		while (true) {
			bool brk = true;
			int chr = (int)(genrand::int32() & 0xFF);

			for (int j = 0; j < i; j++) {
				if (out[j] == chr) {
					brk = false;
					break;
				}
			}

			if (brk) {
				out[i] = chr;
				if (rev) rev[chr] = i;
				break;
			}
		}
	}
}

void udpcrypt_t::init(unsigned int seed)
{
	m_seed = seed;
	create_table(m_seed, m_table, m_table_rev);
}

void udpcrypt_t::encrypt(packet_t* packet)
{
	if (m_seed == 0)
		return;

	for (int i = 0; i < packet->len; ++i)
	{
		char& c = packet->data[i];
		c = (char)m_table[(uint8_t)c];
	}
}

void udpcrypt_t::decrypt(packet_t* packet)
{
	if (m_seed == 0)
		return;

	for (int i = 0; i < packet->len; ++i)
	{
		char& c = packet->data[i];
		c = (char)m_table_rev[(uint8_t)c];
	}
}
