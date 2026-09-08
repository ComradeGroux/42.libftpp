#pragma once

#include "singleton.hpp"

#include <utility>

template <typename TType>
TType*	Singleton<TType>::instance(void)
{
	if (ptr == nullptr)
		throw NoInstanceException();
	return ptr.get();
}

template <typename TType>
template <typename ... TArgs>
void	Singleton<TType>::instantiate(TArgs&& ... p_args)
{
	if (ptr != nullptr)
		throw InstanceAlreadyExistantException();
	ptr.reset(new TType(std::forward<TArgs>(p_args)...));
}
