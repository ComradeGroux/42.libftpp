#include "mathematics/ivector2/ivector2.hpp"

#include <cassert>
#include <iostream>
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

static bool	approxEqual(float a, float b, float eps = 1e-4f)
{
	return std::fabs(a - b) < eps;
}

static void	testConstructionAndMembers(void)
{
	std::cout << "-- testConstructionAndMembers --\n";

	IVector2<int> v{3, 4};

	CHECK(v.x == 3);
	CHECK(v.y == 4);
}

static void	testAddition(void)
{
	std::cout << "-- testAddition --\n";

	IVector2<int> a{1, 2};
	IVector2<int> b{3, 4};
	IVector2<int> c = a + b;

	CHECK(c.x == 4);
	CHECK(c.y == 6);
}

static void	testSubtraction(void)
{
	std::cout << "-- testSubtraction --\n";

	IVector2<int> a{1, 2};
	IVector2<int> b{3, 5};
	IVector2<int> c = b - a;

	CHECK(c.x == 2);
	CHECK(c.y == 3);
}

static void	testMultiplicationComponentWise(void)
{
	std::cout << "-- testMultiplicationComponentWise --\n";

	IVector2<int> a{2, 3};
	IVector2<int> b{4, 5};
	IVector2<int> c = a * b;

	CHECK(c.x == 8);
	CHECK(c.y == 15);
}

static void	testDivisionComponentWise(void)
{
	std::cout << "-- testDivisionComponentWise --\n";

	IVector2<int> a{10, 9};
	IVector2<int> b{2, 3};
	IVector2<int> c = a / b;

	CHECK(c.x == 5);
	CHECK(c.y == 3);
}

static void	testEqualityAndInequality(void)
{
	std::cout << "-- testEqualityAndInequality --\n";

	IVector2<int> a{1, 2};
	IVector2<int> b{1, 2};
	IVector2<int> c{9, 9};

	CHECK(a == b);
	CHECK(!(a == c));
	CHECK(a != c);
	CHECK(!(a != b));
}

static void	testLength(void)
{
	std::cout << "-- testLength --\n";

	IVector2<float> v{3.0f, 4.0f};

	CHECK(approxEqual(v.length(), 5.0f));
}

static void	testNormalize(void)
{
	std::cout << "-- testNormalize --\n";

	IVector2<float> v{3.0f, 4.0f};
	IVector2<float> n = v.normalize();

	CHECK(approxEqual(n.length(), 1.0f));
	CHECK(approxEqual(n.x, 0.6f));
	CHECK(approxEqual(n.y, 0.8f));
}

static void	testDotProduct(void)
{
	std::cout << "-- testDotProduct --\n";

	IVector2<int> a{1, 2};
	IVector2<int> b{3, 4};

	CHECK(a.dot(b) == 11); // 1*3 + 2*4
}

static void	testPerpendicular(void)
{
	std::cout << "-- testPerpendicular --\n";

	IVector2<float> a{1.0f, 0.0f};
	IVector2<float> b{0.0f, 1.0f};
	IVector2<float> c = a.perp();

	std::cout << "  perp result: (" << c.x << ", " << c.y << ")\n";
	CHECK(std::isfinite(c.x));
	CHECK(std::isfinite(c.y));
}

int	main(void)
{
	testConstructionAndMembers();
	testAddition();
	testSubtraction();
	testMultiplicationComponentWise();
	testDivisionComponentWise();
	testEqualityAndInequality();
	testLength();
	testNormalize();
	testDotProduct();
	testPerpendicular();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}