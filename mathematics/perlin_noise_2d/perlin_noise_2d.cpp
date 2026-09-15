#include "perlin_noise_2d.hpp"

#include <cmath>
#include <utility>

PerlinNoise2D::PerlinNoise2D(const std::string& seed)
{
	_hashSeed(seed);
	_fillPermutationTable();
}

/**
 * FNV-1a algorythm
 */
void	PerlinNoise2D::_hashSeed(const std::string& seed)
{
	uint64_t hash = 14695981039346656037ULL;

	for (char c : seed)
	{
		hash ^= static_cast<uint64_t>(c);
		hash *= 1099511628211ULL;
	}

	_seed = hash;
}

void	PerlinNoise2D::_fillPermutationTable(void)
{
	for (int i = 0; i < 256; i++)
		_permutationTable[i] = i;
	
	_fisherYatesShuffle(_seed);

	for (int i = 0; i < 256; i++)
		_permutationTable[i + 256] = _permutationTable[i];
}

void	PerlinNoise2D::_fisherYatesShuffle(uint64_t seed)
{
	for (int i = 255; i > 0; i--)
	{
		seed = seed * 6364136223846793005ull + 1442695040888963407ull;
		int	j = seed % (i + 1);

		std::swap(_permutationTable[i], _permutationTable[j]);
	}
}

float	PerlinNoise2D::_lerp(const float& t, const float& min, const float& max) const noexcept
{
	return min + t * (max - min);
}

float	PerlinNoise2D::_fade(float t) const noexcept
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

const IVector2<float>	PerlinNoise2D::_getNearestCorner(int value) const noexcept
{
	switch (value % 4)
	{
		case 0:
			return IVector2<float>( 1.0f,  1.0f);
		case 1:
			return IVector2<float>( 1.0f, -1.0f);
		case 2:
			return IVector2<float>(-1.0f,  1.0f);
		default:
			return IVector2<float>(-1.0f, -1.0f);
	}
}

static float	positiveModulo(const int& value, const int& mod)
{
	int	res = value % mod;
	if (res < 0)
		return res + mod;
	else
		return res;
}

float	PerlinNoise2D::sample(const float& x, const float& y) const
{
	const float	xf = x - std::floor(x);
	const float yf = y - std::floor(y);

	const IVector2<float>	topLeft(xf - 1.0f, yf - 1.0f);
	const IVector2<float>	topRight(xf, yf - 1.0f);
	const IVector2<float>	bottomLeft(xf - 1.0f, yf);
	const IVector2<float>	bottomRight(xf, yf);

	const int	indexX = positiveModulo(static_cast<int>(std::floor(x)), 256);
	const int	indexY = positiveModulo(static_cast<int>(std::floor(y)), 256);
	const int	topLeftValue = _permutationTable[_permutationTable[indexX + 1] + indexY + 1];
	const int	topRightValue = _permutationTable[_permutationTable[indexX] + indexY + 1];
	const int	bottomLeftValue = _permutationTable[_permutationTable[indexX + 1] + indexY];
	const int	bottomRightValue = _permutationTable[_permutationTable[indexX] + indexY];

	const float	topLeftDot = topLeft.dot(_getNearestCorner(topLeftValue));
	const float	topRightDot = topRight.dot(_getNearestCorner(topRightValue));
	const float	bottomLeftDot = bottomLeft.dot(_getNearestCorner(bottomLeftValue));
	const float	bottomRightDot = bottomRight.dot(_getNearestCorner(bottomRightValue));

	const float	u = _fade(xf);
	const float	v = _fade(yf);
	return _lerp(u, _lerp(v, bottomRightDot, topRightDot), _lerp(v, bottomLeftDot, topLeftDot));
}

float	PerlinNoise2D::operator()(const float& x, const float& y) const
{
	return sample(x, y);
}
