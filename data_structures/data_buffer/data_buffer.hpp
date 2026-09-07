#pragma once

#include <sstream>

class DataBuffer
{
	private:
		DataBuffer(const DataBuffer& src) = delete;
		DataBuffer&	operator=(const DataBuffer& src) = delete;

		std::stringstream	_buffer;

	public:
		DataBuffer(void);
		~DataBuffer(void);

		template <typename TType>
		DataBuffer&	operator<<(const TType& obj);

		template <typename TType>
		DataBuffer&	operator>>(const TType& obj);
};

#include "data_buffer.tpp"
