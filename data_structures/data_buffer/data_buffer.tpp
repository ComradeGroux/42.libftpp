#pragma once

#include "data_buffer.hpp"

#include <stdexcept>
#include <utility>
#include <cstring>

template <typename T, typename = void>
struct is_container : std::false_type {};
template <typename T>
struct is_container<T, std::void_t <
	decltype(std::declval<T>().begin()),
	decltype(std::declval<T>().end())
>> : std::true_type {};

template <typename T, typename = void>
struct has_push_back : std::false_type {};
template <typename T>
struct has_push_back<T, std::void_t <
	decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_insert : std::false_type {};
template <typename T>
struct has_insert<T, std::void_t <
	decltype(std::declval<T>().insert(std::declval<typename T::value_type>()))
>> : std::true_type {};

template <typename T, typename TIndex, typename = void>
struct has_operator_at : std::false_type {};
template <typename T, typename TIndex>
struct has_operator_at<T, TIndex, std::void_t <
	decltype(std::declval<T>()[std::declval<TIndex>()])
>> : std::true_type {};

template <typename T>
struct dependent_false : std::false_type {};

template <typename TType>
DataBuffer&	DataBuffer::operator<<(const TType& obj)
{
	_serialize(obj);
	return *this;
}

template <typename TType>
DataBuffer&	DataBuffer::operator>>(TType& obj)
{
	_deserialize(obj);
	return *this;
}

template <typename TType>
void	DataBuffer::_serialize(const TType& obj)
{
	if constexpr (!is_container<TType>::value && std::is_trivially_copyable_v<TType>)
	{
		const unsigned char*	begin = reinterpret_cast<const unsigned char *>(&obj);
		_buffer.insert(_buffer.end(), begin, begin + sizeof(TType));
	}
	else if constexpr (is_container<TType>::value && has_operator_at<TType, size_t>::value && (has_push_back<TType>::value || has_insert<TType>::value))
	{
		size_t	size = std::distance(obj.begin(), obj.end());
		_serialize(size);

		for (size_t i = 0; i < size; i++)
			_serialize(obj[i]);
	}
	else
		static_assert(dependent_false<TType>::value, "DataBuffer: Unserializable");
}

template <typename T, size_t N>
void	DataBuffer::_serialize(const std::array<T, N>& arr)
{
	size_t	size = arr.size();
	_serialize(size);

	for (size_t i = 0; i < size; i++)
		_serialize(arr[i]);
}

template <typename TType>
void	DataBuffer::_deserialize(TType& obj)
{
	if constexpr (!is_container<TType>::value && std::is_trivially_copyable_v<TType>)
	{
		if (_readOffset + sizeof(TType) > _buffer.size())
			throw NotEnoughByteToDeserializeException();

		std::memcpy(&obj, _buffer.data() + _readOffset, sizeof(TType));
		_readOffset += sizeof(TType);
	}
	else if constexpr (is_container<TType>::value && has_push_back<TType>::value)
	{
		size_t	size = 0;
		_deserialize(size);

		if (std::distance(obj.begin(), obj.end()) < size)
			throw NotEnoughByteToDeserializeException();

		TType	tmp;
		for (size_t i = 0; i < size; i++)
		{
			_deserialize(tmp);
			obj.push_back(tmp);
		}
	}
	else if constexpr (is_container<TType>::value && has_insert<TType>::value)
	{
		size_t	size = 0;
		_deserialize(size);

		if (std::distance(obj.begin(), obj.end()) < size)
			throw NotEnoughByteToDeserializeException();

		TType	tmp;
		for (size_t i = 0; i < size; i++)
		{
			_deserialize(tmp);
			obj.insert(obj.end(), tmp);
		}
	}
	else
		static_assert(dependent_false<TType>::value, "DataBuffer: Undeserializable");
}

template <typename T, size_t N>
void	DataBuffer::_deserialize(std::array<T, N>& arr)
{
	size_t	size = 0;
	_deserialize(size);

	if (N != size)
		throw NotEnoughByteToDeserializeException();

	for (size_t i = 0; i < size; i++)
		_deserialize(arr[i]);
}
