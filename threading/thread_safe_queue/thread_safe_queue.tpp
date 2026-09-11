#pragma once

#include "thread_safe_queue.hpp"

template <typename TType>
void	ThreadSafeQueue<TType>::push_front(const TType& newElement)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	_queue.push_front(newElement);
}

template <typename TType>
void	ThreadSafeQueue<TType>::push_front(TType&& newElement)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	_queue.push_front(std::move(newElement));
}

template <typename TType>
void	ThreadSafeQueue<TType>::push_back(const TType& newElement)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	_queue.push_back(newElement);
}

template <typename TType>
void	ThreadSafeQueue<TType>::push_back(TType&& newElement)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	_queue.push_back(std::move(newElement));
}

template <typename TType>
TType	ThreadSafeQueue<TType>::pop_front(void)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	if (_queue.empty())
		throw ThreadSafeQueueEmptyException();
	TType	res = std::move(_queue.front());
	_queue.pop_front();
	return res;
}

template <typename TType>
TType	ThreadSafeQueue<TType>::pop_back(void)
{
	std::lock_guard<std::mutex>	lock(_mutex);
	if (_queue.empty())
		throw ThreadSafeQueueEmptyException();
	TType	res = std::move(_queue.back());
	_queue.pop_back();
	return res;
}
