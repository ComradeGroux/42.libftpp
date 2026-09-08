#include "memento.hpp"

Memento::Snapshot	Memento::save(void)
{
	Snapshot	state;
	_saveToSnapshot(state);
	return state;
}

void	Memento::load(const Memento::Snapshot& state)
{
	Snapshot	tmp = state;
	_loadFromSnapshot(tmp);
}
