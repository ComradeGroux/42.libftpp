#pragma once

#include <vector>
#include <functional>
#include <atomic>
#include <memory>

#include "../thread/thread.hpp"
#include "../thread_safe_queue/thread_safe_queue.hpp"

class WorkerPool
{
	public:
		class IJob {
			public:
				virtual ~IJob(void) = default;
				virtual void execute(void) = 0;
		};

	private:
		std::vector<std::unique_ptr<Thread>>	_workers;
		ThreadSafeQueue<std::shared_ptr<IJob>>	_jobs;
		std::atomic_bool						_running;

		void	_startThread(void);

		class Job : public IJob {
			private:
				std::function<void()>	_f;
			public:
				Job(std::function<void()> funcToExecute) : _f(funcToExecute) {}
				void	execute(void) { _f(); }
		};

	public:
		WorkerPool(size_t numberWorkers);
		~WorkerPool(void);

		void	addJob(const std::function<void()>& jobToExecute);
		void	addJob(std::shared_ptr<IJob> jobToExecute);
};
