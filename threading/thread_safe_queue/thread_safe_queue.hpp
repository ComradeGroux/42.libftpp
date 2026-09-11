#pragma once

#include <deque>
#include <mutex>
#include <stdexcept>

template <typename TType>
class ThreadSafeQueue
{
	private:
		std::mutex			_mutex;
		std::deque<TType>	_queue;

	public:
		void	push_front(const TType& newElement);
		void	push_front(TType&& newElement);
		void	push_back(const TType& newElement);
		void	push_back(TType&& newElement);
		TType	pop_front(void);
		TType	pop_back(void);

		class ThreadSafeQueueEmptyException : public std::runtime_error
		{
			public: explicit ThreadSafeQueueEmptyException() : runtime_error("ThreadSafeQueue: Queue empty.") {}
		};
};



#include "thread_safe_queue.tpp"
