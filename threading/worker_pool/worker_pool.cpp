#include "worker_pool.hpp"
#include "../../io_stream/thread_safe_iostream.hpp"

#include <iostream>

WorkerPool::WorkerPool(size_t numberWorkers)
{
	_running = true;
	for (size_t i = 0; i < numberWorkers; i++)
	{
		_workers.push_back(std::make_unique<Thread>("Worker" + std::to_string(i), [this]() { _startThread(); }));
		_workers[i].get()->start();
	}
}

WorkerPool::~WorkerPool(void)
{
	_running = false;
	for (std::vector<std::unique_ptr<Thread>>::iterator it = _workers.begin(); it != _workers.end(); it++)
		(*it).get()->stop();
}

void	WorkerPool::_startThread(void)
{
	while (_running)
	{
		try
		{
			std::shared_ptr<IJob>	job = _jobs.pop_front();
			try
			{
				job->execute();
			}
			catch(const std::exception& e)
			{
				threadSafeCerr << e.what() << std::endl;
			}
		}
		catch(const std::exception& e)
		{
			(void)e;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void	WorkerPool::addJob(const std::function<void()>& jobToExecute)
{
	if (jobToExecute)
	{
		Job	job(jobToExecute);
		addJob(std::make_shared<Job>(job));
	}
}

void	WorkerPool::addJob(std::shared_ptr<IJob> jobToExecute)
{
	_jobs.push_back(std::move(jobToExecute));
}
