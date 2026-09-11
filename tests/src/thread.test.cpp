#include "threading/thread/thread.hpp"

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

static void	testOneShotFunctionExecutes(void)
{
	std::cout << "-- testOneShotFunctionExecutes --\n";

	std::atomic<bool> executed{false};
	Thread t("OneShot", [&executed]() { executed = true; });

	t.start();
	t.stop();

	CHECK(executed.load());
}

static void	testStopWithoutStartIsSafe(void)
{
	std::cout << "-- testStopWithoutStartIsSafe --\n";

	std::atomic<bool> executed{false};
	Thread t("NeverStarted", [&executed]() { executed = true; });

	t.stop(); // ne doit pas crasher / bloquer

	CHECK(!executed.load());
}

static void	testStartTwiceDoesNotDuplicateExecution(void)
{
	std::cout << "-- testStartTwiceDoesNotDuplicateExecution --\n";

	std::atomic<int> callCount{0};
	Thread t("DoubleStart", [&callCount]() {
		callCount++;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	});

	t.start();
	t.start(); // doit etre un no-op d'apres l'implementation (deja _running ou deja joignable)
	t.stop();

	CHECK(callCount.load() == 1);
}

static void	testCooperativeLongRunningTaskStopsCleanly(void)
{
	std::cout << "-- testCooperativeLongRunningTaskStopsCleanly --\n";

	// Pattern cooperatif : la fonction boucle tant qu'un flag externe est actif.
	// C'est a l'appelant de positionner ce flag a false AVANT d'appeler stop(),
	// exactement comme WorkerPool le fait avec son propre _running.
	std::atomic<bool> keepRunning{true};
	std::atomic<int> iterations{0};

	Thread t("Looping", [&keepRunning, &iterations]() {
		while (keepRunning.load())
		{
			iterations++;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	});

	t.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	keepRunning = false;
	t.stop(); // doit rejoindre rapidement une fois le flag coupe

	CHECK(iterations.load() > 0);
}

static void	testMultipleThreadsRunConcurrently(void)
{
	std::cout << "-- testMultipleThreadsRunConcurrently --\n";

	std::atomic<int> counter{0};
	Thread t1("A", [&counter]() { counter++; });
	Thread t2("B", [&counter]() { counter++; });
	Thread t3("C", [&counter]() { counter++; });

	t1.start();
	t2.start();
	t3.start();

	t1.stop();
	t2.stop();
	t3.stop();

	CHECK(counter.load() == 3);
}

int	main(void)
{
	testOneShotFunctionExecutes();
	testStopWithoutStartIsSafe();
	testStartTwiceDoesNotDuplicateExecution();
	testCooperativeLongRunningTaskStopsCleanly();
	testMultipleThreadsRunConcurrently();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
