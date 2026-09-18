#include "../../network/message/message.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <cmath>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

static void	testTypeIsStored(void)
{
	std::cout << "-- testTypeIsStored --\n";

	Message msg(42);

	CHECK(msg.type() == 42);
}

static void	testSingleIntRoundtrip(void)
{
	std::cout << "-- testSingleIntRoundtrip --\n";

	Message msg(1);
	msg << 123;

	int result = 0;
	msg >> result;

	CHECK(result == 123);
}

static void	testMultipleValuesPreserveOrder(void)
{
	std::cout << "-- testMultipleValuesPreserveOrder --\n";

	Message msg(2);
	msg << 10 << 3.5f << std::string("hello");

	int a = 0;
	float b = 0.0f;
	std::string c;

	msg >> a >> b >> c;

	CHECK(a == 10);
	CHECK(std::fabs(b - 3.5f) < 1e-4f);
	CHECK(c == "hello");
}

static void	testBoolRoundtrip(void)
{
	std::cout << "-- testBoolRoundtrip --\n";

	Message msg(3);
	msg << true << false;

	bool a = false;
	bool b = true;
	msg >> a >> b;

	CHECK(a == true);
	CHECK(b == false);
}

static void	testDifferentMessagesAreIndependent(void)
{
	std::cout << "-- testDifferentMessagesAreIndependent --\n";

	Message m1(1);
	Message m2(2);

	m1 << 111;
	m2 << 222;

	int v1 = 0;
	int v2 = 0;
	m1 >> v1;
	m2 >> v2;

	CHECK(v1 == 111);
	CHECK(v2 == 222);
	CHECK(m1.type() != m2.type());
}

static void	testChainedInsertionAndExtraction(void)
{
	std::cout << "-- testChainedInsertionAndExtraction --\n";

	// chained << and >> should each return a reference to Message
	Message msg(4);
	int a = 0;
	int b = 0;
	int c = 0;

	msg << 1 << 2 << 3;
	msg >> a >> b >> c;

	CHECK(a == 1);
	CHECK(b == 2);
	CHECK(c == 3);
}

int	main(void)
{
	testTypeIsStored();
	testSingleIntRoundtrip();
	testMultipleValuesPreserveOrder();
	testBoolRoundtrip();
	testDifferentMessagesAreIndependent();
	testChainedInsertionAndExtraction();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
