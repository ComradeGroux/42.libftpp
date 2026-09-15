#pragma once

#include "ivector2.hpp"

#include <cmath>

template <typename TType>
IVector2<TType>::IVector2(void) : x(0), y(0)
{}

template <typename TType>
IVector2<TType>::IVector2(TType x, TType y) : x(x), y(y)
{}

template <typename TType>
IVector2<TType>	IVector2<TType>::operator+(const IVector2<TType>& rhs)
{
	return IVector2<TType>(x + rhs.x, y + rhs.y);
}

template <typename TType>
IVector2<TType>&	IVector2<TType>::operator+=(const IVector2<TType>& rhs)
{
	*this = *this + rhs;
	return *this;
}

template <typename TType>
IVector2<TType>	IVector2<TType>::operator-(const IVector2<TType>& rhs)
{
	return IVector2<TType>(x - rhs.x, y - rhs.y);
}

template <typename TType>
IVector2<TType>&	IVector2<TType>::operator-=(const IVector2<TType>& rhs)
{
	*this = *this - rhs;
	return *this;
}

template <typename TType>
IVector2<TType>	IVector2<TType>::operator*(const IVector2<TType>& rhs)
{
	return IVector2<TType>(x * rhs.x, y * rhs.y);
}

template <typename TType>
IVector2<TType>&	IVector2<TType>::operator*=(const IVector2<TType>& rhs)
{
	*this = *this * rhs;
	return *this;
}

template <typename TType>
IVector2<TType>	IVector2<TType>::operator/(const IVector2<TType>& rhs)
{
	return IVector2<TType>(x / rhs.x, y / rhs.y);
}

template <typename TType>
IVector2<TType>&	IVector2<TType>::operator/=(const IVector2<TType>& rhs)
{
	*this = *this / rhs;
	return *this;
}

template <typename TType>
bool	IVector2<TType>::operator==(const IVector2<TType>& rhs)
{
	return (x == rhs.x && y == rhs.y);
}

template <typename TType>
bool	IVector2<TType>::operator!=(const IVector2<TType>& rhs)
{
	return !(*this == rhs);
}

template <typename TType>
float	IVector2<TType>::length(void) const
{
	return std::sqrt(x * x + y * y);
}

template <typename TType>
IVector2<float>	IVector2<TType>::normalize(void) const
{
	float	len = length();
	IVector2<float>	res;
	if (len > 0.0f)
	{
		res.x = x / len;
		res.y = y / len;
	}
	else
	{
		res.x = 0.0f;
		res.y = 0.0f;
	}

	return res;
}

template <typename TType>
float	IVector2<TType>::dot(const IVector2<TType>& rhs) const noexcept
{
	return (x * rhs.x + y * rhs.y);
}

template <typename TType>
IVector2<TType>	IVector2<TType>::perp(void) const noexcept
{
	IVector2<TType>	res;

	res.x = -y;
	res.y = x;
	return res;
}
