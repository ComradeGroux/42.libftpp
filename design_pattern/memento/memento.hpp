#pragma once

#include "data_structures/data_buffer/data_buffer.hpp"

class Memento
{
	private:

	public:
		class Snapshot
		{
			private:
				DataBuffer	_buffer;

			public:
				template <typename TType>
				friend Snapshot&	operator<<(Snapshot& snapshot, const TType& obj);

				template <typename TType>
				friend Snapshot&	operator>>(Snapshot& snapshot, TType& obj);
		};

		Snapshot	save(void);
		void		load(const Snapshot& state);

	protected:
		virtual void	_saveToSnapshot(Snapshot& snapshot) = 0;
		virtual void	_loadFromSnapshot(Snapshot& snapshot) = 0;
};

#include "snapshot.tpp"
