#pragma once

#include "../../data_structures/data_buffer/data_buffer.hpp"

#include <cstdint>

class Client;
class Server;

class Message
{
	public:
		using Type = int;

		struct Header {
			int			type;
			uint32_t	size;
		};

	private:
		DataBuffer	_data;
		Type		_type;

		void					_setRawData(const unsigned char* data, size_t size);
		const unsigned char*	_rawData(void) const;
		size_t					_rawSize(void) const;

		friend Client;
		friend Server;

	public:
		Message(Type type);

		int	type(void) const noexcept;

		template <typename TType>
		friend Message&	operator<<(Message& msg, const TType& data);

		template <typename TType>
		friend Message&	operator>>(Message& msg, TType& data);
};

#include "message.tpp"
