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

	const unsigned char*	bytes = reinterpret_cast<const unsigned char *>(_buffer.data() + _readOffset);
	str.assign(reinterpret_cast<const char *>(bytes), size);
	_readOffset += size;
}

size_t	DataBuffer::rawSize(void) const
{
	return _buffer.size();
}

const unsigned char*	DataBuffer::rawData(void) const
{
	return _buffer.data();
}

void	DataBuffer::setRawData(const unsigned char* data, size_t size)
{
	_buffer.assign(data, data + size);
	_readOffset = 0;
}
