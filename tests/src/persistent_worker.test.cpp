#include "threading/persistent_worker/persistent_worker.hpp"

#include <cassert>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

template <typename Predicate>
static bool	waitUntil(Predicate predicate, int timeoutMs = 2000)
{
	const int stepMs = 5;
	int waited = 0;
	while (!predicate())
	{
		if (waited >= timeoutMs)
			return false;
		std::this_thread::sleep_for(std::chrono::milliseconds(stepMs));
		waited += stepMs;
	}
	return true;
}

static void	testTaskExecutesRepeatedly(void)
{
	std::cout << "-- testTaskExecutesRepeatedly --\n";

	PersistentWorker worker;
	std::atomic<int> counter{0};

	worker.addTask("counter", [&counter]() { counter++; });

	// Une tache "en boucle" doit s'executer plusieurs fois, pas une seule.
	CHECK(waitUntil([&counter]() { return counter.load() >= 5; }));
}

static void	testRemoveTaskStopsFurtherExecution(void)
{
	std::cout << "-- testRemoveTaskStopsFurtherExecution --\n";

	PersistentWorker worker;
	std::atomic<int> counter{0};

	worker.addTask("stoppable", [&counter]() { counter++; });

	CHECK(waitUntil([&counter]() { return counter.load() >= 3; }));

	worker.removeTask("stoppable");
	int countAtRemoval = counter.load();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	int countAfterWait = counter.load();

	CHECK(countAfterWait == countAtRemoval);
}

static void	testMultipleIndependentTasksRunConcurrently(void)
{
	std::cout << "-- testMultipleIndependentTasksRunConcurrently --\n";

	PersistentWorker worker;
	std::atomic<int> counterA{0};
	std::atomic<int> counterB{0};

	worker.addTask("taskA", [&counterA]() { counterA++; });
	worker.addTask("taskB", [&counterB]() { counterB++; });

	CHECK(waitUntil([&counterA]() { return counterA.load() >= 3; }));
	CHECK(waitUntil([&counterB]() { return counterB.load() >= 3; }));
}

static void	testRemoveOneTaskDoesNotAffectOthers(void)
{
	std::cout << "-- testRemoveOneTaskDoesNotAffectOthers --\n";

	PersistentWorker worker;
	std::atomic<int> counterA{0};
	std::atomic<int> counterB{0};

	worker.addTask("keep", [&counterA]() { counterA++; });
	worker.addTask("remove", [&counterB]() { counterB++; });

	CHECK(waitUntil([&counterB]() { return counterB.load() >= 3; }));
	worker.removeTask("remove");

	int counterAAtRemoval = counterA.load();
	CHECK(waitUntil([&counterA, counterAAtRemoval]() { return counterA.load() > counterAAtRemoval; }));
}

static void	testRemoveNonexistentTaskDoesNotCrash(void)
{
	std::cout << "-- testRemoveNonexistentTaskDoesNotCrash --\n";

	PersistentWorker worker;

	worker.removeTask("does_not_exist");

	CHECK(true);
}

static void	testDestructorStopsCleanly(void)
{
	std::cout << "-- testDestructorStopsCleanly --\n";

	// Si le worker ne s'arrete pas correctement a la destruction, ce test bloquera
	// indefiniment (a observer si le binaire ne se termine jamais).
	{
		PersistentWorker worker;
		std::atomic<int> counter{0};
		worker.addTask("scoped", [&counter]() { counter++; });
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	} // destruction ici

	CHECK(true);
}

int	main(void)
{
	testTaskExecutesRepeatedly();
	testRemoveTaskStopsFurtherExecution();
	testMultipleIndependentTasksRunConcurrently();
	testRemoveOneTaskDoesNotAffectOthers();
	testRemoveNonexistentTaskDoesNotCrash();
	testDestructorStopsCleanly();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
