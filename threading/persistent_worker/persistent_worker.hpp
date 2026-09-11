#pragma once

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

class PersistentWorker
{
	public:
		class ITask {
			public:
				virtual ~ITask(void) = default;
				virtual void	execute(void) = 0;
		};

	private:
		std::thread												_thread;
		std::mutex												_mutex;
		std::unordered_map<std::string, std::unique_ptr<ITask>>	_tasks;
		std::atomic_bool										_running;

		void	_loop(void);

		class Task : public ITask {
			private:
				std::function<void()>	_f;
			public:
				Task(std::function<void()> funcToExectute) : _f(funcToExectute) {}
				void	execute(void) { _f(); }
		};

	public:
		PersistentWorker(void);
		~PersistentWorker(void);

		void	addTask(const std::string& name, const std::function<void()>& jobToExecute);
		void	addTask(const std::string& name, std::unique_ptr<ITask> jobToExecute);

		void	removeTask(const std::string& name);
};
