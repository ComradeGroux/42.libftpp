#include "io_stream/thread_safe_iostream.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
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

// Petit outil pour capturer ce qui est ecrit sur std::cout pendant la duree de vie de l'objet.
class CoutCapture
{
	private:
		std::ostringstream	_buffer;
		std::streambuf*	_old;

	public:
		CoutCapture(void) : _old(std::cout.rdbuf(_buffer.rdbuf())) {}
		~CoutCapture(void) { std::cout.rdbuf(_old); }
		std::string	str(void) const { return _buffer.str(); }
};

// Petit outil pour injecter un contenu simule dans std::cin pendant la duree de vie de l'objet.
class CinInjector
{
	private:
		std::istringstream	_buffer;
		std::streambuf*	_old;

	public:
		explicit CinInjector(const std::string& content) : _buffer(content), _old(std::cin.rdbuf(_buffer.rdbuf())) {}
		~CinInjector(void) { std::cin.rdbuf(_old); }
};

static void	testBasicOutputAppearsWithPrefix(void)
{
	std::cout << "-- testBasicOutputAppearsWithPrefix --\n";

	ThreadSafeIOStream stream;
	stream.setPrefix("[Test] ");

	CoutCapture capture;
	stream << "Hello" << std::endl;
	std::string result = capture.str();

	CHECK(result.find("[Test]") != std::string::npos);
	CHECK(result.find("Hello") != std::string::npos);
}

static void	testNoOutputBeforeEndl(void)
{
	std::cout << "-- testNoOutputBeforeEndl --\n";

	ThreadSafeIOStream stream;
	stream.setPrefix("[Buffered] ");

	CoutCapture capture;
	stream << "part1 " << "part2 " << 42; // pas de endl, pas de flush attendu

	CHECK(capture.str().empty());
}

static void	testEndlFlushesAccumulatedContent(void)
{
	std::cout << "-- testEndlFlushesAccumulatedContent --\n";

	ThreadSafeIOStream stream;
	stream.setPrefix("[Buffered] ");

	CoutCapture capture;
	stream << "part1 " << "part2 " << 42 << std::endl;
	std::string result = capture.str();

	CHECK(result.find("part1 part2 42") != std::string::npos);
}

static void	testMultipleTypesInsertion(void)
{
	std::cout << "-- testMultipleTypesInsertion --\n";

	ThreadSafeIOStream stream;
	CoutCapture capture;

	stream << "int=" << 7 << " double=" << 3.5 << " bool=" << true << std::endl;
	std::string result = capture.str();

	CHECK(result.find("int=7") != std::string::npos);
	CHECK(result.find("double=3.5") != std::string::npos);
}

static void	testFormattingManipulatorApplies(void)
{
	std::cout << "-- testFormattingManipulatorApplies --\n";

	ThreadSafeIOStream stream;
	CoutCapture capture;

	stream << std::hex << 255 << std::endl;
	std::string result = capture.str();

	CHECK(result.find("ff") != std::string::npos);
}

static void	testUnitbufFlushesWithoutEndl(void)
{
	std::cout << "-- testUnitbufFlushesWithoutEndl --\n";

	ThreadSafeIOStream stream;
	stream.setPrefix("[Unit] ");

	CoutCapture capture;
	stream << std::unitbuf << "immediate";
	std::string result = capture.str();

	CHECK(result.find("immediate") != std::string::npos);
}

static void	testPromptReadsInjectedInput(void)
{
	std::cout << "-- testPromptReadsInjectedInput --\n";

	ThreadSafeIOStream stream;
	CinInjector inject("42\n");
	CoutCapture capture;

	int dest = 0;
	stream.prompt("Enter a number: ", dest);

	CHECK(dest == 42);
	CHECK(capture.str().find("Enter a number:") != std::string::npos);
}

static void	testOperatorGreaterThanReadsMultipleValuesSameLine(void)
{
	std::cout << "-- testOperatorGreaterThanReadsMultipleValuesSameLine --\n";

	ThreadSafeIOStream stream;
	CinInjector inject("10 20\n");

	int a = 0, b = 0;
	stream >> a >> b;

	CHECK(a == 10);
	CHECK(b == 20);
}

static void	testEachThreadHasOwnPrefix(void)
{
	std::cout << "-- testEachThreadHasOwnPrefix --\n";

	CoutCapture capture;

	std::thread t1([]() {
		threadSafeCout.setPrefix("[T1] ");
		threadSafeCout << "message-from-t1" << std::endl;
	});
	std::thread t2([]() {
		threadSafeCout.setPrefix("[T2] ");
		threadSafeCout << "message-from-t2" << std::endl;
	});

	t1.join();
	t2.join();

	std::string result = capture.str();

	CHECK(result.find("[T1]") != std::string::npos);
	CHECK(result.find("message-from-t1") != std::string::npos);
	CHECK(result.find("[T2]") != std::string::npos);
	CHECK(result.find("message-from-t2") != std::string::npos);
}

static void	testConcurrentWritesDoNotCrash(void)
{
	std::cout << "-- testConcurrentWritesDoNotCrash --\n";

	// On ne verifie pas l'entrelacement exact (difficile a garantir),
	// seulement l'absence de crash / corruption evidente sous charge concurrente.
	CoutCapture capture;

	std::vector<std::thread> threads;
	for (int i = 0; i < 8; i++)
	{
		threads.emplace_back([i]() {
			threadSafeCout.setPrefix("[W" + std::to_string(i) + "] ");
			for (int j = 0; j < 20; j++)
				threadSafeCout << "line " << j << std::endl;
		});
	}
	for (auto& t : threads)
		t.join();

	CHECK(true); // si on arrive ici sans crash, c'est le principal
}

int	main(void)
{
	testBasicOutputAppearsWithPrefix();
	testNoOutputBeforeEndl();
	testEndlFlushesAccumulatedContent();
	testMultipleTypesInsertion();
	testFormattingManipulatorApplies();
	testUnitbufFlushesWithoutEndl();
	testPromptReadsInjectedInput();
	testOperatorGreaterThanReadsMultipleValuesSameLine();
	testEachThreadHasOwnPrefix();
	testConcurrentWritesDoNotCrash();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
