#pragma once

#include <functional>
#include <exception>
#include <map>
#include <vector>

template <typename TState>
class StateMachine
{
	private:
		std::vector<TState>											_states;
		std::map<TState, std::function<void()>>						_actions;
		std::map<std::pair<TState, TState>, std::function<void()>>	_transitions;

		TState	_currentState = static_cast<TState>(0);

		bool	_isStateKnown(const TState& state);

	public:
		void	addState(const TState& state);
		void	addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda);
		void	addAction(const TState& state, const std::function<void()>& lambda);

		void	transitionTo(const TState& state);
		void	update(void);

		class UndefinedTransitionException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "StateMachine: Undefined transition"; }
		};
		class UndefinedActionException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "StateMachine: Undefined action"; }
		};
		class UnknownStateException : public std::exception {
			public:
				const char*	what(void) const noexcept { return "StateMachine: Unknown state"; }
		};
};

#include "state_machine.tpp"
