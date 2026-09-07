#pragma once

#include "pool.hpp"

#include <algorithm>
#include <iostream>

template <typename TType>
void	Pool<TType>::_growPool(const size_t& newSize)
{
	for (size_t toAllocate = newSize - _totalAllocated; toAllocate > 0; toAllocate--)
	{
		TType*	raw = static_cast<TType *>(operator new(sizeof(TType)));

		_availableObjects.push_back(raw);
		_allocatedObjects.push_back(std::unique_ptr<TType, RawDeleter<TType>>(raw));

		_totalAllocated++;
	}
}

template <typename TType>
void	Pool<TType>::_shrinkPool(const size_t& newSize) 
{
	size_t	toRemove = _totalAllocated - newSize;
	while (toRemove > 0 && !_allocatedObjects.empty())
	{
		if (_availableObjects.empty())
			throw NoAvailableObjectsToRemoveException();

		TType*	obj = _availableObjects.back();
		_availableObjects.pop_back();

		typename std::vector<std::unique_ptr<TType, RawDeleter<TType>>>::iterator	it = std::find_if(
			_allocatedObjects.begin(), _allocatedObjects.end(),
			[obj](const std::unique_ptr<TType, RawDeleter<TType>>& ptr) { return ptr.get() == obj; }
		);
		
		_allocatedObjects.erase(it);
		toRemove--;
		_totalAllocated--;
	}

	if (toRemove > 0)
		throw NoAllocatedObjectsToRemoveException();
}

template <typename TType>
void	Pool<TType>::resize(const size_t& numberOfObjectStored)
{
	if (numberOfObjectStored > _totalAllocated)
		_growPool(numberOfObjectStored);
	else if (numberOfObjectStored < _totalAllocated)
		_shrinkPool(numberOfObjectStored);
}

template <typename TType>
template <typename ... TArgs>
typename Pool<TType>::Object	Pool<TType>::acquire(TArgs&& ... p_args)
{
	if (_availableObjects.empty())
		throw NoAvailableObjectsException();

	TType*	obj = _availableObjects.back();
	_availableObjects.pop_back();

	new(obj) TType(std::forward<TArgs>(p_args)...);

	return Object(this, obj);
}

template <typename TType>
Pool<TType>::Object::Object(Pool<TType>* pool, TType* value) : _pool(pool), _value(value)
{
}

template <typename TType>
Pool<TType>::Object::~Object(void)
{
	_value->~TType();
	_pool->_availableObjects.push_back(_value);
}

template <typename TType>
TType*	Pool<TType>::Object::operator->(void)
{
	return _value;
}
