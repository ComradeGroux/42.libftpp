#include "threading/thread_safe_queue/thread_safe_queue.hpp"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <numeric>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

static void	testPushBackPopFrontFIFO(void)
{
	std::cout << "-- testPushBackPopFrontFIFO --\n";

	ThreadSafeQueue<int> q;
	q.push_back(1);
	q.push_back(2);
	q.push_back(3);

	CHECK(q.pop_front() == 1);
	CHECK(q.pop_front() == 2);
	CHECK(q.pop_front() == 3);
}

static void	testPushFrontOrdering(void)
{
	std::cout << "-- testPushFrontOrdering --\n";

	ThreadSafeQueue<int> q;
	q.push_back(1);
	q.push_front(0); // doit passer devant

	CHECK(q.pop_front() == 0);
	CHECK(q.pop_front() == 1);
}

static void	testPopBack(void)
{
	std::cout << "-- testPopBack --\n";

	ThreadSafeQueue<int> q;
	q.push_back(1);
	q.push_back(2);
	q.push_back(3);

	CHECK(q.pop_back() == 3);
	CHECK(q.pop_back() == 2);
	CHECK(q.pop_back() == 1);
}

static void	testPopOnEmptyQueueThrows(void)
{
	std::cout << "-- testPopOnEmptyQueueThrows --\n";

	ThreadSafeQueue<int> q;
	bool threwFront = false;
	bool threwBack = false;

	try { q.pop_front(); }
	catch (const ThreadSafeQueue<int>::ThreadSafeQueueEmptyException&) { threwFront = true; }

	try { q.pop_back(); }
	catch (const ThreadSafeQueue<int>::ThreadSafeQueueEmptyException&) { threwBack = true; }

	CHECK(threwFront);
	CHECK(threwBack);
}

static void	testLvalueOverloadCopiesSourceUnchanged(void)
{
	std::cout << "-- testLvalueOverloadCopiesSourceUnchanged --\n";

	ThreadSafeQueue<std::string> q;
	std::string source = "hello";

	q.push_back(source); // overload const&, ne doit pas vider 'source'

	CHECK(source == "hello"); // toujours valide apres l'appel
	CHECK(q.pop_front() == "hello");
}

static void	testMoveOnlyTypeWorks(void)
{
	std::cout << "-- testMoveOnlyTypeWorks --\n";

	ThreadSafeQueue<std::unique_ptr<int>> q;

	q.push_back(std::make_unique<int>(42));
	q.push_front(std::make_unique<int>(1));

	std::unique_ptr<int> first = q.pop_front();
	std::unique_ptr<int> second = q.pop_front();

	CHECK(first != nullptr && *first == 1);
	CHECK(second != nullptr && *second == 42);
}

static void	testConcurrentProducersConsumers(void)
{
	std::cout << "-- testConcurrentProducersConsumers --\n";

	ThreadSafeQueue<int> q;
	constexpr int itemsPerProducer = 500;
	constexpr int producerCount = 4;

	std::vector<std::thread> producers;
	for (int p = 0; p < producerCount; p++)
	{
		producers.emplace_back([&q, p]() {
			for (int i = 0; i < itemsPerProducer; i++)
				q.push_back(p * itemsPerProducer + i);
		});
	}
	for (auto& t : producers)
		t.join();

	std::atomic<long long> sum{0};
	std::atomic<int> consumedCount{0};
	const int totalItems = itemsPerProducer * producerCount;

	std::vector<std::thread> consumers;
	for (int c = 0; c < producerCount; c++)
	{
		consumers.emplace_back([&q, &sum, &consumedCount, totalItems]() {
			while (true)
			{
				int current = consumedCount.fetch_add(1);
				if (current >= totalItems)
				{
					consumedCount.fetch_sub(1);
					break;
				}
				try
				{
					int value = q.pop_front();
					sum += value;
				}
				catch (const ThreadSafeQueue<int>::ThreadSafeQueueEmptyException&)
				{
					consumedCount.fetch_sub(1);
				}
			}
		});
	}
	for (auto& t : consumers)
		t.join();

	long long expectedSum = 0;
	for (int i = 0; i < totalItems; i++)
		expectedSum += i;

	CHECK(sum.load() == expectedSum);
}

int	main(void)
{
	testPushBackPopFrontFIFO();
	testPushFrontOrdering();
	testPopBack();
	testPopOnEmptyQueueThrows();
	testLvalueOverloadCopiesSourceUnchanged();
	testMoveOnlyTypeWorks();
	testConcurrentProducersConsumers();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
