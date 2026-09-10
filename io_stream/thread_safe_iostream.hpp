#pragma once

#include <mutex>
#include <string>
#include <iostream>
#include <sstream>

class ThreadSafeIOStream
{
	private:
		static std::mutex	_mutex;
		std::string			_prefix;
		std::ostringstream	_output;
		std::istringstream	_input;
		bool				_unitBuff = false;
		bool				_unitBuffInput = false;

		void	_flush(void);

	public:
		~ThreadSafeIOStream(void);

		template <typename TType>
		ThreadSafeIOStream&	operator<<(const TType& data);
		template <typename T>
		ThreadSafeIOStream& operator<<(std::basic_ios<T>& (manipulator)(std::basic_ios<T>&));
		ThreadSafeIOStream& operator<<(std::ios_base& (manipulator)(std::ios_base&));
		ThreadSafeIOStream&	operator<<(std::ostream& (manipulator)(std::ostream&));

		template <typename TType>
		ThreadSafeIOStream&	operator>>(TType& data);
		template <typename T>
		ThreadSafeIOStream&	operator>>(std::basic_ios<T>& (manipulator)(std::basic_ios<T>&));
		ThreadSafeIOStream& operator>>(std::ios_base& (manipulator)(std::ios_base&));
		ThreadSafeIOStream&	operator>>(std::istream& (manipulator)(std::istream&));

		void	setPrefix(const std::string& prefix);
		template <typename T>
		void	prompt(const std::string& question, T& dest);

		class InputFailedException : public std::runtime_error {
			public:
				explicit InputFailedException() : runtime_error("ThreadSafeIOStream: Input's extraction failed") {}
		};
};

inline thread_local ThreadSafeIOStream	threadSafeCout;

#include "thread_safe_iostream.tpp"
