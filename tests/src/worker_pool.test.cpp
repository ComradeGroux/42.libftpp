#include "threading/worker_pool/worker_pool.hpp"

#include <cassert>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <memory>
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

// Attend jusqu'a 'timeoutMs' millisecondes que 'predicate' devienne vrai,
// pour eviter des flakiness sur un pool asynchrone sans figer un delai fixe.
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

// Job concret de test pour la surcharge addJob(std::unique_ptr<IJob>)
class IncrementJob : public WorkerPool::IJob
{
	private:
		std::atomic<int>& _counter;

	public:
		explicit IncrementJob(std::atomic<int>& counter) : _counter(counter) {}
		void	execute(void) override { _counter++; }
};

static void	testSingleFunctionJobExecutes(void)
{
	std::cout << "-- testSingleFunctionJobExecutes --\n";

	WorkerPool pool(2);
	std::atomic<bool> executed{false};

	pool.addJob([&executed]() { executed = true; });

	CHECK(waitUntil([&executed]() { return executed.load(); }));
}

static void	testIJobOverloadExecutes(void)
{
	std::cout << "-- testIJobOverloadExecutes --\n";

	WorkerPool pool(2);
	std::atomic<int> counter{0};

	pool.addJob(std::make_unique<IncrementJob>(counter));

	CHECK(waitUntil([&counter]() { return counter.load() == 1; }));
}

static void	testManyJobsAllExecuteExactlyOnce(void)
{
	std::cout << "-- testManyJobsAllExecuteExactlyOnce --\n";

	WorkerPool pool(4);
	std::atomic<int> counter{0};
	const int jobCount = 200;

	for (int i = 0; i < jobCount; i++)
		pool.addJob([&counter]() { counter++; });

	CHECK(waitUntil([&counter, jobCount]() { return counter.load() == jobCount; }));
}

static void	testJobThrowingDoesNotStopOtherJobs(void)
{
	std::cout << "-- testJobThrowingDoesNotStopOtherJobs --\n";

	WorkerPool pool(2);
	std::atomic<int> successCount{0};

	pool.addJob([]() { throw std::runtime_error("boom"); });
	pool.addJob([&successCount]() { successCount++; });
	pool.addJob([&successCount]() { successCount++; });

	CHECK(waitUntil([&successCount]() { return successCount.load() == 2; }));
}

static void	testMixedFunctionAndIJobJobs(void)
{
	std::cout << "-- testMixedFunctionAndIJobJobs --\n";

	WorkerPool pool(3);
	std::atomic<int> counter{0};

	pool.addJob([&counter]() { counter++; });
	pool.addJob(std::make_unique<IncrementJob>(counter));
	pool.addJob([&counter]() { counter++; });
	pool.addJob(std::make_unique<IncrementJob>(counter));

	CHECK(waitUntil([&counter]() { return counter.load() == 4; }));
}

static void	testDestructorDoesNotHang(void)
{
	std::cout << "-- testDestructorDoesNotHang --\n";

	// Si le pool ne s'arrete pas correctement, ce test bloquera indefiniment
	// (a observer manuellement si le binaire de test ne se termine jamais).
	{
		WorkerPool pool(4);
		pool.addJob([]() { std::this_thread::sleep_for(std::chrono::milliseconds(20)); });
	} // destruction ici

	CHECK(true); // si on atteint cette ligne, la destruction n'a pas bloque
}

int	main(void)
{
	testSingleFunctionJobExecutes();
	testIJobOverloadExecutes();
	testManyJobsAllExecuteExactlyOnce();
	testJobThrowingDoesNotStopOtherJobs();
	testMixedFunctionAndIJobJobs();
	testDestructorDoesNotHang();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
