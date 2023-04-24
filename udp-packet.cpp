#include "main.h"

bool ipacket_t::read(void* dst, size_t n)
{
	if ((pos + n) > len)
		return false;

	memcpy(dst, (data + pos), n);
	pos += n;

	return true;
}

bool ipacket_t::read_string(std::string& val)
{
	uint32_t size;
	if (!read<uint32_t>(size))
		return false;

	if ((size + pos) > len)
		return false;

	val.resize(size);
	return read(&val[0], size);
}

// ------------------------------------------------------------------------- //

void opacket_t::write(const void* src, size_t n)
{
	data.append((const char*)src, n);
}

void opacket_t::write_string(const std::string& in)
{
	write<uint32_t>(in.size());
	write(in.data(), in.size());
}

ENetPacket* opacket_t::to_enet(enet_uint32 flags)
{
	return enet_packet_create(data.data(), data.size(), flags);
}
