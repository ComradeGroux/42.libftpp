#pragma once

#include "message.hpp"

template <typename TType>
Message&	operator<<(Message& msg, const TType& data)
{
	msg._data << data;
	return msg;
}

template <typename TType>
Message&	operator>>(Message& msg, TType& data)
{
	msg._data >> data;
	return msg;
}
