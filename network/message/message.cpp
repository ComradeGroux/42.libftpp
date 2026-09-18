#include "message.hpp"

Message::Message(int type) : _type(type)
{}

int	Message::type(void) const noexcept
{
	return _type;
}

void	Message::_setRawData(const unsigned char* data, size_t size)
{
	_data.setRawData(data, size);
}

const unsigned char*	Message::_rawData(void) const
{
	return _data.rawData();
}

size_t	Message::_rawSize(void) const
{
	return _data.rawSize();
}
