#include "mathematics/perlin_noise_2d/perlin_noise_2d.hpp"

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

static void	testSampleIsDeterministic(void)
{
	std::cout << "-- testSampleIsDeterministic --\n";

	PerlinNoise2D perlin;
	float v1 = perlin.sample(1.5f, 2.5f);
	float v2 = perlin.sample(1.5f, 2.5f);

	CHECK(v1 == v2);
}

static void	testSampleIsFiniteAndBounded(void)
{
	std::cout << "-- testSampleIsFiniteAndBounded --\n";

	PerlinNoise2D perlin;

	for (float x = 0.0f; x < 5.0f; x += 0.37f)
	{
		for (float y = 0.0f; y < 5.0f; y += 0.53f)
		{
			float v = perlin.sample(x, y);
			CHECK(std::isfinite(v));
			// classic Perlin noise stays roughly within [-1, 1];
			// giving a bit of margin for different implementations.
			CHECK(v > -2.0f && v < 2.0f);
		}
	}
}

static void	testDifferentCoordinatesLikelyGiveDifferentValues(void)
{
	std::cout << "-- testDifferentCoordinatesLikelyGiveDifferentValues --\n";

	PerlinNoise2D perlin;
	float v1 = perlin.sample(1.23f, 4.56f);
	float v2 = perlin.sample(50.1f, 88.9f);

	CHECK(v1 != v2);
}

static void	testNoiseIsContinuous(void)
{
	std::cout << "-- testNoiseIsContinuous --\n";

	PerlinNoise2D perlin;
	float	v1  = perlin.sample(2.999f, 1.5f);
	float	v2 = perlin.sample(3.001f, 1.5f);

	// a tiny step in coordinates should only produce a tiny change in value
	CHECK(std::fabs(v1 - v2) < 0.05f);
}

static void	testGridOfSamplesDoesNotCrash(void)
{
	std::cout << "-- testGridOfSamplesDoesNotCrash --\n";

	PerlinNoise2D perlin;

	for (int x = -10; x <= 10; x++)
	{
		for (int y = -10; y <= 10; y++)
			(void)perlin.sample(static_cast<float>(x) * 0.1f, static_cast<float>(y) * 0.1f);
	}

	CHECK(true); // reaching here without crash is the main point
}

int	main(void)
{
	testSampleIsDeterministic();
	testSampleIsFiniteAndBounded();
	testDifferentCoordinatesLikelyGiveDifferentValues();
	testNoiseIsContinuous();
	testGridOfSamplesDoesNotCrash();

	std::cout << "\n" << (g_testCount - g_failCount) << "/" << g_testCount << " checks passed.\n";
	return g_failCount == 0 ? 0 : 1;
}
