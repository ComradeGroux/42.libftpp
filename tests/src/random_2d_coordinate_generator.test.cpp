#include "mathematics/random_2d_coordinate_generator/random_2d_coordinate_generator.hpp"

#include <cassert>
#include <iostream>
#include <set>

static int g_testCount = 0;
static int g_failCount = 0;

#define CHECK(cond) do { \
	g_testCount++; \
	if (!(cond)) { \
		g_failCount++; \
		std::cerr << "  [FAIL] " << #cond << " (line " << __LINE__ << ")\n"; \
	} \
} while (0)

static void	testSeedIsStable(void)
{
	std::cout << "-- testSeedIsStable --\n";

	Random2DCoordinateGenerator gen;
	long long s1 = gen.seed();
	long long s2 = gen.seed();

	CHECK(s1 == s2);
}

static void	testSameCoordinatesGiveSameResult(void)
{
	std::cout << "-- testSameCoordinatesGiveSameResult --\n";

	Random2DCoordinateGenerator gen;
	long long v1 = gen(5, 7);
	long long v2 = gen(5, 7);

	CHECK(v1 == v2);
}

static void	testDifferentCoordinatesLikelyGiveDifferentResult(void)
{
	std::cout << "-- testDifferentCoordinatesLikelyGiveDifferentResult --\n";

	Random2DCoordinateGenerator gen;
	long long v1 = gen(5, 7);
	long long v2 = gen(5, 8);
	long long v3 = gen(6, 7);

	// probabilistic: a decent hash should not collide on close inputs
	CHECK(v1 != v2);
	CHECK(v1 != v3);
}

static void	testManyCoordinatesProduceVariedResults(void)
{
	std::cout << "-- testManyCoordinatesProduceVariedResults --\n";

	Random2DCoordinateGenerator gen;
	std::set<long long> results;

	for (long long x = 0; x < 20; x++)
	{
		for (long long y = 0; y < 20; y++)
			results.insert(gen(x, y));
	}

	// Expect a good spread: at least 90% of the 400 samples are unique.
	CHECK(results.size() > 360);
}

static void	testDifferentInstancesLikelyHaveDifferentSeeds(void)
{
	std::cout << "-- testDifferentInstancesLikelyHaveDifferentSeeds --\n";

	Random2DCoordinateGenerator genA;
	Random2DCoordinateGenerator genB;

	CHECK(genA.seed() != genB.seed());
}

static void	testNegativeCoordinatesDoNotCrash(void)
{
	std::cout << "-- testNegativeCoordinatesDoNotCrash --\n";

	Random2DCoordinateGenerator gen;
	long long v1 = gen(-5, -7);
	long long v2 = gen(-5, -7);

	CHECK(v1 == v2); // still deterministic
	CHECK(true); // reaching here without crash/UB is the main point
}

int	main(void)
{
	testSeedIsStable();
	testSameCoordinatesGiveSameResult();
	testDifferentCoordinatesLikelyGiveDifferentResult();
	testManyCoordinatesProduceVariedResults();
	testDifferentInstancesLikelyHaveDifferentSeeds();
	testNegativeCoordinatesDoNotCrash();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
