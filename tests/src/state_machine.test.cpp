#include "design_pattern/state_machine/state_machine.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

enum class DoorState
{
	Closed,
	Open,
	Locked
};

static void	testTransitionExecutesLambda(void)
{
	std::cout << "-- testTransitionExecutesLambda --\n";

	StateMachine<DoorState> sm;
	bool transitionCalled = false;

	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Open);
	sm.addTransition(DoorState::Closed, DoorState::Open, [&transitionCalled]() { transitionCalled = true; });

	sm.transitionTo(DoorState::Open);

	CHECK(transitionCalled);
}

static void	testActionRunsOnUpdate(void)
{
	std::cout << "-- testActionRunsOnUpdate --\n";

	StateMachine<DoorState> sm;
	int actionCallCount = 0;

	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Open);
	sm.addTransition(DoorState::Closed, DoorState::Open, []() {});
	sm.addAction(DoorState::Open, [&actionCallCount]() { actionCallCount++; });

	sm.transitionTo(DoorState::Open);
	sm.update();
	sm.update();
	sm.update();

	CHECK(actionCallCount == 3);
}

static void	testActionOnlyRunsForCurrentState(void)
{
	std::cout << "-- testActionOnlyRunsForCurrentState --\n";

	StateMachine<DoorState> sm;
	int closedActionCount = 0;
	int openActionCount = 0;

	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Open);
	sm.addTransition(DoorState::Closed, DoorState::Open, []() {});
	sm.addAction(DoorState::Closed, [&closedActionCount]() { closedActionCount++; });
	sm.addAction(DoorState::Open, [&openActionCount]() { openActionCount++; });

	sm.update(); // etat initial suppose etre Closed (premier etat ajoute) ou un etat par defaut
	sm.transitionTo(DoorState::Open);
	sm.update();

	CHECK(openActionCount == 1);
	// Note: la valeur exacte de closedActionCount depend de l'etat initial choisi
	// par ton implementation -- verifie manuellement si ce test a du sens pour toi.
}

static void	testUndefinedTransitionThrows(void)
{
	std::cout << "-- testUndefinedTransitionThrows --\n";

	StateMachine<DoorState> sm;
	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Locked);
	// Aucune transition Closed -> Locked n'est definie

	bool threw = false;
	try
	{
		sm.transitionTo(DoorState::Locked);
	}
	catch (const std::exception&)
	{
		threw = true;
	}

	CHECK(threw);
}

static void	testUpdateWithoutActionThrows(void)
{
	std::cout << "-- testUpdateWithoutActionThrows --\n";

	StateMachine<DoorState> sm;
	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Open);
	sm.addTransition(DoorState::Closed, DoorState::Open, []() {});
	// Aucune action definie pour Open

	sm.transitionTo(DoorState::Open);

	bool threw = false;
	try
	{
		sm.update();
	}
	catch (const std::exception&)
	{
		threw = true;
	}

	CHECK(threw);
}

static void	testChainedTransitions(void)
{
	std::cout << "-- testChainedTransitions --\n";

	StateMachine<DoorState> sm;
	std::string path;

	sm.addState(DoorState::Closed);
	sm.addState(DoorState::Open);
	sm.addState(DoorState::Locked);

	sm.addTransition(DoorState::Closed, DoorState::Open, [&path]() { path += "C->O;"; });
	sm.addTransition(DoorState::Open, DoorState::Locked, [&path]() { path += "O->L;"; });
	sm.addTransition(DoorState::Locked, DoorState::Closed, [&path]() { path += "L->C;"; });

	sm.transitionTo(DoorState::Open);
	sm.transitionTo(DoorState::Locked);
	sm.transitionTo(DoorState::Closed);

	CHECK(path == "C->O;O->L;L->C;");
}

int	main(void)
{
	testTransitionExecutesLambda();
	testActionRunsOnUpdate();
	testActionOnlyRunsForCurrentState();
	testUndefinedTransitionThrows();
	testUpdateWithoutActionThrows();
	testChainedTransitions();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
