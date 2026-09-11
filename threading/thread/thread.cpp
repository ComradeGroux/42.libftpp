#include "thread.hpp"

#include "../../io_stream/thread_safe_iostream.hpp"

Thread::Thread(const std::string& name, std::function<void()> functToExecute)
{
	_name = name;
	_f = std::move(functToExecute);
	_running = false;
}

void	Thread::start(void)
{
	if (_running || _thread.joinable())
		return;

	_running = true;
	_thread = std::thread([this]() {
		threadSafeCout.setPrefix("[" + _name + "]");
		threadSafeCerr.setPrefix("[" + _name + "]");
		_f();
	});
}

void	Thread::stop(void)
{
	if (!_running || !_thread.joinable())
		return;

	_running = false;
	_thread.join();
}
