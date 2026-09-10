#include "data_structures/data_buffer/data_buffer.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <array>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

// Type trivially copyable custom, pour verifier que ce n'est pas limite aux types "built-in"
struct Vec3
{
	float x;
	float y;
	float z;

	bool operator==(const Vec3& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

static void	testTrivialTypes(void)
{
	std::cout << "-- testTrivialTypes --\n";

	DataBuffer buf;
	int i = 42;
	double d = 3.14159;
	char c = 'A';
	bool b = true;
	Vec3 v{1.0f, 2.0f, 3.0f};

	buf << i << d << c << b << v;

	int i2 = 0;
	double d2 = 0.0;
	char c2 = 0;
	bool b2 = false;
	Vec3 v2{};

	buf >> i2 >> d2 >> c2 >> b2 >> v2;

	CHECK(i2 == 42);
	CHECK(d2 == 3.14159);
	CHECK(c2 == 'A');
	CHECK(b2 == true);
	CHECK(v2 == v);
}

static void	testStringRoundTrip(void)
{
	std::cout << "-- testStringRoundTrip --\n";

	DataBuffer buf;
	std::string s1 = "hello world with spaces";
	std::string s2 = "";
	std::string s3 = "another string after an empty one";

	buf << s1 << s2 << s3;

	std::string r1, r2, r3;
	buf >> r1 >> r2 >> r3;

	CHECK(r1 == s1);
	CHECK(r2 == s2);
	CHECK(r3 == s3);
}

static void	testMixedOrder(void)
{
	std::cout << "-- testMixedOrder --\n";

	// Verifie que le format respecte bien un ordre FIFO (le premier ecrit est le premier lu)
	DataBuffer buf;
	buf << 1 << std::string("two") << 3.0;

	int a;
	std::string b;
	double c;
	buf >> a >> b >> c;

	CHECK(a == 1);
	CHECK(b == "two");
	CHECK(c == 3.0);
}

static void	testVectorOfInt(void)
{
	std::cout << "-- testVectorOfInt (necessite has_push_back branch implementee) --\n";

	DataBuffer buf;
	std::vector<int> original{1, 2, 3, 4, 5};

	buf << original;

	std::vector<int> result;
	buf >> result;

	CHECK(result == original);
}

static void	testVectorOfString(void)
{
	std::cout << "-- testVectorOfString --\n";

	DataBuffer buf;
	std::vector<std::string> original{"alpha", "beta", "gamma"};

	buf << original;

	std::vector<std::string> result;
	buf >> result;

	CHECK(result == original);
}

static void	testNestedVector(void)
{
	std::cout << "-- testNestedVector --\n";

	DataBuffer buf;
	std::vector<std::vector<int>> original{{1, 2}, {3, 4, 5}, {}};

	buf << original;

	std::vector<std::vector<int>> result;
	buf >> result;

	CHECK(result == original);
}

static void	testFixedArray(void)
{
	std::cout << "-- testFixedArray --\n";

	DataBuffer buf;
	std::array<int, 4> original{10, 20, 30, 40};

	buf << original;

	std::array<int, 4> result{};
	buf >> result;

	CHECK(result == original);
}

static void	testEmptyBufferThrows(void)
{
	std::cout << "-- testEmptyBufferThrows --\n";

	DataBuffer buf;
	int dest = 0;
	bool threw = false;

	try
	{
		buf >> dest;
	}
	catch (const DataBuffer::NotEnoughByteToDeserializeException&)
	{
		threw = true;
	}
	CHECK(threw);
}

static void	testReadPastEndThrows(void)
{
	std::cout << "-- testReadPastEndThrows --\n";

	DataBuffer buf;
	buf << 42; // seulement un int dans le buffer

	int a = 0;
	int b = 0;
	bool threw = false;

	try
	{
		buf >> a; // ok
		buf >> b; // plus rien a lire, doit lever une exception
	}
	catch (const DataBuffer::NotEnoughByteToDeserializeException&)
	{
		threw = true;
	}
	CHECK(a == 42);
	CHECK(threw);
}

static void	testWriteAfterPartialRead(void)
{
	std::cout << "-- testWriteAfterPartialRead --\n";

	// Verifie qu'on peut ecrire dans le buffer apres avoir deja lu partiellement,
	// et que la lecture continue correctement a la suite
	DataBuffer buf;
	buf << 1 << 2;

	int a = 0;
	buf >> a;
	CHECK(a == 1);

	buf << 3;

	int b = 0, c = 0;
	buf >> b >> c;
	CHECK(b == 2);
	CHECK(c == 3);
}

int	main(void)
{
	testTrivialTypes();
	testStringRoundTrip();
	testMixedOrder();
	testVectorOfInt();
	testVectorOfString();
	testNestedVector();
	testFixedArray();
	testEmptyBufferThrows();
	testReadPastEndThrows();
	testWriteAfterPartialRead();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}