#pragma once

class Random2DCoordinateGenerator
{
	private:
		long long	_seed;

	public:
		Random2DCoordinateGenerator(void);
		Random2DCoordinateGenerator(const long long& seed);

		long long	seed(void) const;
		void		setSeed(const long long& newSeed) noexcept;

		long long	operator()(const long long& x, const long long& y) const;
};
