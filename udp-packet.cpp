#include "main.h"

bool packet_t::Write(const void* src, len_t n)
{
	if ((len + n) > packet_max_size)
		return false;

	memcpy((data + len), src, n);
	len += n;

	return true;
}

bool packet_t::Read(void* dst, len_t n)
{
	if ((pos + n) > len)
		return false;

	memcpy(dst, (data + pos), n);
	pos += n;

	return true;
}

bool packet_t::WriteString(const std::string& in)
{
	len_t len = static_cast<len_t>(in.size());
	return (Write<len_t>(len) && Write(in.data(), len));
}

bool packet_t::ReadString(std::string& val)
{
	len_t size;
	if (!Read<len_t>(size))
		return false;

	if ((size + pos) > len)
		return false;

	val.resize(size);
	return Read(&val[0], size);
}
