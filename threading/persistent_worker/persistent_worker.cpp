#include "persistent_worker.hpp"

PersistentWorker::PersistentWorker(void)
{
	_running = true;
	_thread = std::thread([this]() { _loop(); });
}

PersistentWorker::~PersistentWorker(void)
{
	_running = false;
	if (_thread.joinable())
		_thread.join();
}

void	PersistentWorker::_loop(void)
{
	while (_running)
	{
		{
			std::lock_guard<std::mutex>	lock(_mutex);
			for (const auto& [name, func] : _tasks)
			{
				if (!_running)
					return;
				func.get()->execute();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void	PersistentWorker::addTask(const std::string& name, const std::function<void()>& jobToExecute)
{
	if (!name.empty() && jobToExecute)
	{
		Task	task(jobToExecute);
		addTask(name, std::make_unique<Task>(task));
	}
}

void	PersistentWorker::addTask(const std::string& name, std::unique_ptr<ITask> jobToExecute)
{
	if (!name.empty())
	{
		std::lock_guard<std::mutex>	lock(_mutex);
		_tasks[name] = std::move(jobToExecute);
	}
}

void	PersistentWorker::removeTask(const std::string& name)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	_tasks.erase(name);
}
