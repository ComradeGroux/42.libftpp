#pragma once

#include "observer.hpp"

template <typename TEvent>
void	Observer<TEvent>::subscribe(const TEvent& event, const std::function<void()>& lambda)
{
	_eventsMap[event].push_back(lambda);
}

template <typename TEvent>
void	Observer<TEvent>::notify(const TEvent& event)
{
	if (_eventsMap.find(event) == _eventsMap.end())
		return;

	for (std::vector<std::function<void()>>::iterator it = _eventsMap[event].begin(); it != _eventsMap[event].end(); it++)
		(*it)();
}
