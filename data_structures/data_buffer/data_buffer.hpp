#pragma once

#include <vector>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <array>

class DataBuffer
{
	private:
		std::vector<unsigned char>	_buffer;
		size_t						_readOffset = 0;

		void	_serialize(const std::string& str);
		template <typename TType>
		void	_serialize(const TType& obj);
		template <typename T, size_t N>
		void	_serialize(const std::array<T, N>& arr);

		void	_deserialize(std::string& str);
		template <typename TType>
		void	_deserialize(TType& obj);
		template <typename T, size_t N>
		void	_deserialize(std::array<T, N>& arr);

	public:
		template <typename TType>
		DataBuffer&	operator<<(const TType& obj);

		template <typename TType>
		DataBuffer&	operator>>(TType& obj);

		class NotEnoughByteToDeserializeException : public std::runtime_error
		{
			public: explicit NotEnoughByteToDeserializeException() : runtime_error("DataBuffer: Not enough byte to deserialize.") {}
		};
};

#include "data_buffer.tpp"
