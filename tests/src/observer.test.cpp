#include "design_pattern/observer/observer.hpp"
 
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
 
static int g_testCount = 0;
static int g_failCount = 0;
 
#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)
 
enum class GameEvent
{
	PlayerDied,
	PlayerScored,
	LevelUp
};
 
static void	testSingleSubscriberIsNotified(void)
{
	std::cout << "-- testSingleSubscriberIsNotified --\n";
 
	Observer<GameEvent> obs;
	bool called = false;
 
	obs.subscribe(GameEvent::PlayerScored, [&called]() { called = true; });
	obs.notify(GameEvent::PlayerScored);
 
	CHECK(called);
}
 
static void	testUnrelatedEventDoesNotTrigger(void)
{
	std::cout << "-- testUnrelatedEventDoesNotTrigger --\n";
 
	Observer<GameEvent> obs;
	bool called = false;
 
	obs.subscribe(GameEvent::PlayerScored, [&called]() { called = true; });
	obs.notify(GameEvent::PlayerDied); // different event
 
	CHECK(!called);
}
 
static void	testMultipleSubscribersSameEvent(void)
{
	std::cout << "-- testMultipleSubscribersSameEvent --\n";
 
	Observer<GameEvent> obs;
	int callCount = 0;
 
	obs.subscribe(GameEvent::LevelUp, [&callCount]() { callCount++; });
	obs.subscribe(GameEvent::LevelUp, [&callCount]() { callCount++; });
	obs.subscribe(GameEvent::LevelUp, [&callCount]() { callCount++; });
 
	obs.notify(GameEvent::LevelUp);
 
	CHECK(callCount == 3);
}
 
static void	testNotifyMultipleTimes(void)
{
	std::cout << "-- testNotifyMultipleTimes --\n";
 
	Observer<GameEvent> obs;
	int callCount = 0;
 
	obs.subscribe(GameEvent::PlayerDied, [&callCount]() { callCount++; });
 
	obs.notify(GameEvent::PlayerDied);
	obs.notify(GameEvent::PlayerDied);
	obs.notify(GameEvent::PlayerDied);
 
	CHECK(callCount == 3);
}
 
static void	testNotifyWithNoSubscribersDoesNotCrash(void)
{
	std::cout << "-- testNotifyWithNoSubscribersDoesNotCrash --\n";
 
	Observer<GameEvent> obs;
	obs.notify(GameEvent::PlayerScored); // ne doit rien faire, ne doit pas crasher
 
	CHECK(true); // si on arrive ici sans crash, c'est deja une bonne nouvelle
}
 
static void	testCapturedStateInLambda(void)
{
	std::cout << "-- testCapturedStateInLambda --\n";
 
	Observer<GameEvent> obs;
	std::vector<std::string> log;
 
	obs.subscribe(GameEvent::PlayerScored, [&log]() { log.push_back("scored"); });
	obs.subscribe(GameEvent::PlayerDied, [&log]() { log.push_back("died"); });
 
	obs.notify(GameEvent::PlayerScored);
	obs.notify(GameEvent::PlayerDied);
	obs.notify(GameEvent::PlayerScored);
 
	CHECK(log.size() == 3);
	CHECK(log[0] == "scored");
	CHECK(log[1] == "died");
	CHECK(log[2] == "scored");
}
 
int	main(void)
{
	testSingleSubscriberIsNotified();
	testUnrelatedEventDoesNotTrigger();
	testMultipleSubscribersSameEvent();
	testNotifyMultipleTimes();
	testNotifyWithNoSubscribersDoesNotCrash();
	testCapturedStateInLambda();
 
	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
 
