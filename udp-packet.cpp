#include "main.h"

packet_t::packet_t(ENetPacket* ptr)
	: packet(ptr), size(-1), pos(0)
{

}

packet_t::packet_t(packet_id id, enet_uint32 flags)
	: packet(enet_packet_create(0, 0, flags | ENET_PACKET_FLAG_NO_ALLOCATE)), size(32), pos(0)
{
	packet->data = (enet_uint8*)malloc(size);
	packet->freeCallback = free_callback;
	write<packet_id>(id);
}

packet_t::~packet_t()
{
	if (packet->referenceCount == 0)
		enet_packet_destroy(packet);
}

void packet_t::realloc(size_t _size)
{
	size_t new_size = size * 2;
	if (new_size < (size + _size))
		new_size = (size + _size);
	packet->data = (enet_uint8*)::realloc(packet->data, new_size);
	size = new_size;
}

void packet_t::free_callback(void* data)
{
	ENetPacket* packet = reinterpret_cast<ENetPacket*>(data);
	free(packet->data);
}

bool packet_t::read(void* dst, size_t n)
{
	if ((pos + n) > packet->dataLength)
		return false;

	memcpy(dst, (packet->data + pos), n);
	pos += n;

	return true;
}

bool packet_t::read_string(std::string& val)
{
	uint32_t len;
	if (!read<uint32_t>(len))
		return false;

	if ((len + pos) > packet->dataLength)
		return false;

	val.resize(len);
	return read(&val[0], len);
}

void packet_t::write(const void* src, size_t n)
{
	if (size == (size_t)(-1))
		return;

	if ((pos + n) > size)
		realloc(n);

	memcpy((packet->data + pos), src, n);
	packet->dataLength = (pos += n);
}

void packet_t::write_string(const std::string& in)
{
	write<uint32_t>((uint32_t)in.size());
	write(in.data(), in.size());
}
