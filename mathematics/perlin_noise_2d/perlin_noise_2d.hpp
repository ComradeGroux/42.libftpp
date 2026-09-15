#pragma once

#include "../ivector2/ivector2.hpp"

#include <array>
#include <string>
#include <cstdint>

class PerlinNoise2D
{
	private:
		uint64_t				_seed;
		std::array<int, 512>	_permutationTable;

		void	_hashSeed(const std::string& seed);

		void					_fillPermutationTable(void);
		void					_fisherYatesShuffle(uint64_t seed);
		const IVector2<float>	_getNearestCorner(int value) const noexcept;

		float	_lerp(const float& t, const float& min, const float& max) const noexcept;
		float	_fade(float t) const noexcept;

	public:
		PerlinNoise2D(const std::string& seed = "I love computer graphics");

		float	sample(const float& x, const float& y) const;
		float	operator()(const float& x, const float& y) const;
};
