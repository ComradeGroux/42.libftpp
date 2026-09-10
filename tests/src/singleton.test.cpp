#include "design_pattern/singleton/singleton.hpp"

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

// Classe geree en singleton : un logger simple avec un prefixe configure une seule fois
class Logger : public Singleton<Logger>
{
	public:
		std::string prefix;
		int messageCount = 0;

		void	log(const std::string& msg)
		{
			messageCount++;
			(void)msg;
		}

	private:
		explicit Logger(std::string prefix_) : prefix(std::move(prefix_)) {}

		friend class Singleton<Logger>;
};

static void	testInstantiateThenInstance(void)
{
	std::cout << "-- testInstantiateThenInstance --\n";

	Logger::instantiate(std::string("[APP] "));
	Logger* logger = Logger::instance();

	CHECK(logger != nullptr);
	CHECK(logger->prefix == "[APP] ");
}

static void	testInstanceReturnsSameAddress(void)
{
	std::cout << "-- testInstanceReturnsSameAddress --\n";

	Logger* first = Logger::instance();
	Logger* second = Logger::instance();

	CHECK(first == second);
}

static void	testStateIsSharedAcrossCalls(void)
{
	std::cout << "-- testStateIsSharedAcrossCalls --\n";

	Logger::instance()->log("hello");
	Logger::instance()->log("world");

	CHECK(Logger::instance()->messageCount == 2);
}

static void	testDoubleInstantiateThrows(void)
{
	std::cout << "-- testDoubleInstantiateThrows --\n";

	// A ce stade, Logger a deja ete instancie par testInstantiateThenInstance().
	bool threw = false;
	try
	{
		Logger::instantiate(std::string("[SHOULD_FAIL] "));
	}
	catch (const std::exception&)
	{
		threw = true;
	}

	CHECK(threw);
	// L'etat precedent ne doit pas avoir ete ecrase par la tentative ratee
	CHECK(Logger::instance()->prefix == "[APP] ");
}

int	main(void)
{
	testInstantiateThenInstance();
	testInstanceReturnsSameAddress();
	testStateIsSharedAcrossCalls();
	testDoubleInstantiateThrows();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
