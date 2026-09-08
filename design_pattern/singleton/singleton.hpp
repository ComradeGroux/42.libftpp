#pragma once

#include <memory>
#include <exception>

template <typename TType>
class Singleton
{
	private:
		static std::unique_ptr<TType>	ptr;

	public:
		static TType*	instance(void);	

		template <typename ... TArgs>
		static void	instantiate(TArgs&& ... p_args);

		class NoInstanceException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "Singleton: Instance is not created yet"; }
		};
		class InstanceAlreadyExistantException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "Singleton: Instance already created"; }
		};

	protected:
		Singleton(void) = default;
		virtual ~Singleton(void) = default;
};

template <typename TType>
std::unique_ptr<TType> Singleton<TType>::ptr = nullptr;

#include "singleton.tpp"
