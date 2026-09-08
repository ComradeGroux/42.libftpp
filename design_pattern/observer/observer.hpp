#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

template <typename TEvent>
class Observer
{
	private:
		std::unordered_map<TEvent, std::vector<std::function<void()>>>	_eventsMap;

	public:
		void	subscribe(const TEvent& event, const std::function<void()>& lambda);
		void	notify(const TEvent& event);
};

#include "observer.tpp"
