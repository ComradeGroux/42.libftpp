#include "data_buffer.hpp"

void	DataBuffer::_serialize(const std::string& str)
{
	size_t	size = str.length();
	_serialize(size);

	const unsigned char*	begin = reinterpret_cast<const unsigned char *>(str.data());
	const unsigned char*	end = reinterpret_cast<const unsigned char *>(str.data() + size);

	_buffer.insert(_buffer.end(), begin, end);
}

void	DataBuffer::_deserialize(std::string& str)
{
	size_t	size = 0;
	_deserialize(size);

	if (_buffer.size() < size)
		throw NotEnoughByteToDeserializeException();

	const unsigned char*	bytes = reinterpret_cast<const unsigned char *>(_buffer.data());
	str.assign(reinterpret_cast<const char *>(bytes), size);
	_readOffset += size;
}
