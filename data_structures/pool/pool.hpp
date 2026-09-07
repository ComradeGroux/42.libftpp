#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

template <typename TType>
struct RawDeleter
{
	void	operator()(TType* p) const
	{
		operator delete(p);
	}
};

template <typename TType>
class Pool
{
	private:
		std::vector<TType*>										_availableObjects;
		std::vector<std::unique_ptr<TType, RawDeleter<TType>>>	_allocatedObjects;
		size_t													_totalAllocated = 0;

		void	_growPool(const size_t& newSize);
		void	_shrinkPool(const size_t& newSize);

	public:
		class Object
		{
			private:
				Object(Pool<TType>* pool, TType* value);
				Object(const Object& src) = delete;
				Object&	operator=(const Object& src) = delete;

				Pool<TType>*	_pool;
				TType*			_value;

				friend class Pool;
			
			public:
				~Object(void);
				TType*	operator->(void);
		};
		void	resize(const size_t& numberOfObjectStored);

		template <typename ... TArgs>
		Object	acquire(TArgs&& ... p_args);

		class NoAvailableObjectsException : public std::runtime_error
		{
			public: explicit NoAvailableObjectsException() : runtime_error("Pool: No available objects.") {}
		};
		class NoAvailableObjectsToRemoveException : public std::runtime_error
		{
			public: explicit NoAvailableObjectsToRemoveException() : runtime_error("Pool: No available objects to remove.") {}
		};
		class NoAllocatedObjectsToRemoveException : public std::runtime_error
		{
			public: explicit NoAllocatedObjectsToRemoveException() : runtime_error("Pool: No allocated objects to remove.") {}
		};
};

#include "pool.tpp"
