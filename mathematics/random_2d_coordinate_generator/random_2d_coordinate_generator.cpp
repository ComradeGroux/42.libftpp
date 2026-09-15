#include "random_2d_coordinate_generator.hpp"

#include <random>

Random2DCoordinateGenerator::Random2DCoordinateGenerator(void)
{
	_seed = std::random_device{}();
}

Random2DCoordinateGenerator::Random2DCoordinateGenerator(const long long& seed) : _seed(seed)
{}

long long	Random2DCoordinateGenerator::seed(void) const
{
	return _seed;
}

void		Random2DCoordinateGenerator::setSeed(const long long& newSeed) noexcept
{
	_seed = newSeed;
}

long long	Random2DCoordinateGenerator::operator()(const long long& x, const long long& y) const
{
	std::seed_seq	seq{_seed, x, y};
	std::mt19937_64	generator(seq);

	std::uniform_int_distribution<long long>	distribution(0, std::numeric_limits<long long>::max());

	return distribution(generator);
}
