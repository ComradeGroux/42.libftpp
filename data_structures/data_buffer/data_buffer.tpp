#pragma once

#include "data_buffer.hpp"

template <typename TType>
DataBuffer&	DataBuffer::operator<<(const TType& obj)
{
	_buffer << obj << " ";
	return *this;
}

template <typename TType>
DataBuffer&	DataBuffer::operator>>(const TType& obj)
{
	if (_buffer.eof())
		throw std::runtime_error("Buffer is empty");

	_buffer >> obj;
	return *this;
}
