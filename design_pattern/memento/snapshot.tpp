#pragma once

#include "memento.hpp"

template <typename TType>
Memento::Snapshot&	operator<<(Memento::Snapshot& snapshot, const TType& obj)
{
	snapshot._buffer << obj;
	return snapshot;
}

template <typename TType>
Memento::Snapshot&	operator>>(Memento::Snapshot& snapshot, TType& obj)
{
	snapshot._buffer >> obj;
	return snapshot;
}
