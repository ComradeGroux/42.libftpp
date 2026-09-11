#pragma once

#include <thread>
#include <functional>
#include <string>
#include <atomic>

class Thread
{
	private:
		std::thread				_thread;
		std::string				_name;
		std::function<void()>	_f;
		std::atomic_bool		_running;

	public:
		Thread(const std::string& name, std::function<void()> functToExecute);

		void	start(void);
		void	stop(void);
};
