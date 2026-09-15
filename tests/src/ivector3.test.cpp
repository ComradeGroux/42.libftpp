#include "mathematics/ivector3/ivector3.hpp"

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

	IVector3<int> v{1, 2, 3};

	CHECK(v.x == 1);
	CHECK(v.y == 2);
	CHECK(v.z == 3);
}

static void	testAddition(void)
{
	std::cout << "-- testAddition --\n";

	IVector3<int> a{1, 2, 3};
	IVector3<int> b{4, 5, 6};
	IVector3<int> c = a + b;

	CHECK(c.x == 5);
	CHECK(c.y == 7);
	CHECK(c.z == 9);
}

static void	testSubtraction(void)
{
	std::cout << "-- testSubtraction --\n";

	IVector3<int> a{5, 7, 9};
	IVector3<int> b{1, 2, 3};
	IVector3<int> c = a - b;

	CHECK(c.x == 4);
	CHECK(c.y == 5);
	CHECK(c.z == 6);
}

static void	testMultiplicationComponentWise(void)
{
	std::cout << "-- testMultiplicationComponentWise --\n";

	IVector3<int> a{2, 3, 4};
	IVector3<int> b{5, 6, 7};
	IVector3<int> c = a * b;

	CHECK(c.x == 10);
	CHECK(c.y == 18);
	CHECK(c.z == 28);
}

static void	testDivisionComponentWise(void)
{
	std::cout << "-- testDivisionComponentWise --\n";

	IVector3<int> a{10, 20, 30};
	IVector3<int> b{2, 4, 5};
	IVector3<int> c = a / b;

	CHECK(c.x == 5);
	CHECK(c.y == 5);
	CHECK(c.z == 6);
}

static void	testEqualityAndInequality(void)
{
	std::cout << "-- testEqualityAndInequality --\n";

	IVector3<int> a{1, 2, 3};
	IVector3<int> b{1, 2, 3};
	IVector3<int> c{9, 9, 9};

	CHECK(a == b);
	CHECK(!(a == c));
	CHECK(a != c);
	CHECK(!(a != b));
}

static void	testLength(void)
{
	std::cout << "-- testLength --\n";

	IVector3<float> v{2.0f, 3.0f, 6.0f}; // 2-3-6 -> length 7

	CHECK(approxEqual(v.length(), 7.0f));
}

static void	testNormalize(void)
{
	std::cout << "-- testNormalize --\n";

	IVector3<float> v{2.0f, 3.0f, 6.0f};
	IVector3<float> n = v.normalize();

	CHECK(approxEqual(n.length(), 1.0f));
}

static void	testDotProduct(void)
{
	std::cout << "-- testDotProduct --\n";

	// NOTE: assuming dot(other), see ivector2 tests for rationale.
	IVector3<int> a{1, 2, 3};
	IVector3<int> b{4, 5, 6};

	CHECK(a.dot(b) == 32); // 1*4 + 2*5 + 3*6
}

static void	testCrossProduct(void)
{
	std::cout << "-- testCrossProduct --\n";

	// The 3D cross product is well-defined:
	// cross(a, b) = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx)
	IVector3<float> x{1.0f, 0.0f, 0.0f};
	IVector3<float> y{0.0f, 1.0f, 0.0f};
	IVector3<float> z = x.cross(y);

	CHECK(approxEqual(z.x, 0.0f));
	CHECK(approxEqual(z.y, 0.0f));
	CHECK(approxEqual(z.z, 1.0f));
}

static void	testCrossProductAntiCommutative(void)
{
	std::cout << "-- testCrossProductAntiCommutative --\n";

	IVector3<float> a{1.0f, 2.0f, 3.0f};
	IVector3<float> b{4.0f, 5.0f, 6.0f};

	IVector3<float> ab = a.cross(b);
	IVector3<float> ba = b.cross(a);

	CHECK(approxEqual(ab.x, -ba.x));
	CHECK(approxEqual(ab.y, -ba.y));
	CHECK(approxEqual(ab.z, -ba.z));
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
	testCrossProduct();
	testCrossProductAntiCommutative();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
