#pragma once

#include "thread_safe_iostream.hpp"

template <typename TType>
ThreadSafeIOStream&	ThreadSafeIOStream::operator<<(const TType& data)
{
	_output << data;

	if (_unitBuff)
		_flush();

	return *this;
}

template <typename T>
ThreadSafeIOStream& ThreadSafeIOStream::operator<<(std::basic_ios<T>& (manipulator)(std::basic_ios<T>&))
{
	manipulator(_output);

	if (_unitBuff)
		_flush();

	return *this;
}

template <typename TType>
ThreadSafeIOStream&	ThreadSafeIOStream::operator>>(TType& data)
{
	while (true)
	{
		if (_input >> data)
			return *this;
		
		_input.clear();
		
		std::string	line;
		{
			std::lock_guard<std::mutex>	lock(_mutex);
			if (!std::getline(std::cin, line))
				throw InputFailedException();
		}

		_input.str(line);
	}
}

template <typename T>
ThreadSafeIOStream&	ThreadSafeIOStream::operator>>(std::basic_ios<T>& (manipulator)(std::basic_ios<T>&))
{
	manipulator(_input);
	return *this;
}

/**
 * Note: Displaying the question and reading the answer are not guaranteed to be atomic with respect to other threads — A concurrent log entry could be interleaved.
 */
template <typename T>
void	ThreadSafeIOStream::prompt(const std::string& question, T& dest)
{
	*this << question << std::flush;
	*this >> dest;
}
