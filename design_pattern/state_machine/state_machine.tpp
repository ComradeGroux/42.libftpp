#pragma once

#include "state_machine.hpp"

template <typename TState>
bool	StateMachine<TState>::_isStateKnown(const TState& state)
{
	return (std::find(_states.begin(), _states.end(), state) == _states.end());
}

template <typename TState>
void	StateMachine<TState>::addState(const TState& state)
{
	if (!_isStateKnown(state))
		_states.push_back(state);
}

template <typename TState>
void	StateMachine<TState>::addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda)
{
	if (!_isStateKnown(startState) || !_isStateKnown(finalState))
		throw UnknownStateException();

	_transitions[std::pair(startState, finalState)] = lambda;
}

template <typename TState>
void	StateMachine<TState>::addAction(const TState& state, const std::function<void()>& lambda)
{
	if (!_isStateKnown(state))
		throw UnknownStateException();

	_actions[state] = lambda;
}

template <typename TState>
void	StateMachine<TState>::transitionTo(const TState& state)
{
	if (!_isStateKnown(state))
		throw UnknownStateException();

	if (_transitions.find(std::pair(_currentState, state)) == _transitions.end())
		throw UndefinedTransitionException();

	if (_transitions[std::pair(_currentState, state)])
		_transitions[std::pair(_currentState, state)]();

	_currentState = state;
}

template <typename TState>
void	StateMachine<TState>::update(void)
{
	if (_actions.find(_currentState) == _actions.end())
		throw UndefinedActionException();

	if (_actions[_currentState])
		_actions[_currentState]();
}
