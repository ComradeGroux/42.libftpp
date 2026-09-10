#include "thread_safe_iostream.hpp"

std::mutex	ThreadSafeIOStream::_mutex;

ThreadSafeIOStream::~ThreadSafeIOStream(void)
{
	if (!_output.str().empty())
		_flush();
}

void	ThreadSafeIOStream::_flush(void)
{
	const std::string	outputStr = _prefix + _output.str();
	_output.str("");
	_output.clear();

	std::lock_guard<std::mutex>	lock(_mutex);
	std::cout << outputStr << std::flush;
}

void	ThreadSafeIOStream::setPrefix(const std::string& prefix)
{
	_prefix = prefix;
}

ThreadSafeIOStream& ThreadSafeIOStream::operator<<(std::ios_base& (manipulator)(std::ios_base&))
{
	if (manipulator == static_cast<std::ios_base& (*)(std::ios_base&)>(std::unitbuf))
		_unitBuff = true;
	else if (manipulator == static_cast<std::ios_base& (*)(std::ios_base&)>(std::nounitbuf))
		_unitBuff = false;
	else
	{
		manipulator(_output);
		if (_unitBuff)
			_flush();
	}

	return *this;
}

ThreadSafeIOStream&	ThreadSafeIOStream::operator<<(std::ostream& (manipulator)(std::ostream&))
{
	if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
	{
		_output << "\n";
		_flush();
	}
	else if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::flush))
		_flush();
	else
	{
		manipulator(_output);
		if (_unitBuff)
			_flush();
	}

	return *this;
}

ThreadSafeIOStream& ThreadSafeIOStream::operator>>(std::ios_base& (manipulator)(std::ios_base&))
{
	manipulator(_input);
	return *this;
}

ThreadSafeIOStream&	ThreadSafeIOStream::operator>>(std::istream& (manipulator)(std::istream&))
{
	manipulator(_input);
	return *this;
}
