#pragma once

#include "ivector3.hpp"

#include <cmath>

template <typename TType>
IVector3<TType>::IVector3(void) : x(0), y(0), z(0)
{}

template <typename TType>
IVector3<TType>::IVector3(TType x, TType y, TType z) : x(x), y(y), z(z)
{}

template <typename TType>
IVector3<TType>	IVector3<TType>::operator+(const IVector3<TType>& rhs)
{
	return IVector3<TType>(x + rhs.x, y + rhs.y, z + rhs.z);
}

template <typename TType>
IVector3<TType>&	IVector3<TType>::operator+=(const IVector3<TType>& rhs)
{
	*this = *this + rhs;
	return *this;
}

template <typename TType>
IVector3<TType>	IVector3<TType>::operator-(const IVector3<TType>& rhs)
{
	return IVector3<TType>(x - rhs.x, y - rhs.y, z - rhs.z);
}

template <typename TType>
IVector3<TType>&	IVector3<TType>::operator-=(const IVector3<TType>& rhs)
{
	*this = *this - rhs;
	return *this;
}

template <typename TType>
IVector3<TType>	IVector3<TType>::operator*(const IVector3<TType>& rhs)
{
	return IVector3<TType>(x * rhs.x, y * rhs.y, z * rhs.z);
}

template <typename TType>
IVector3<TType>&	IVector3<TType>::operator*=(const IVector3<TType>& rhs)
{
	*this = *this * rhs;
	return *this;
}

template <typename TType>
IVector3<TType>	IVector3<TType>::operator/(const IVector3<TType>& rhs)
{
	return IVector3<TType>(x / rhs.x, y / rhs.y, z / rhs.z);
}

template <typename TType>
IVector3<TType>&	IVector3<TType>::operator/=(const IVector3<TType>& rhs)
{
	*this = *this / rhs;
	return *this;
}

template <typename TType>
bool	IVector3<TType>::operator==(const IVector3<TType>& rhs)
{
	return (x == rhs.x && y == rhs.y && z == rhs.z);
}

template <typename TType>
bool	IVector3<TType>::operator!=(const IVector3<TType>& rhs)
{
	return !(*this == rhs);
}

template <typename TType>
float	IVector3<TType>::length(void)
{
	return std::sqrt(x * x + y * y + z * z);
}

template <typename TType>
IVector3<float>	IVector3<TType>::normalize(void)
{
	float	len = length();
	IVector3<float>	res;
	if (len > 0.0f)
	{
		res.x = x / len;
		res.y = y / len;
		res.z = z / len;
	}
	else
	{
		res.x = 0.0f;
		res.y = 0.0f;
		res.z = 0.0f;
	}

	return res;
}

template <typename TType>
float	IVector3<TType>::dot(const IVector3& rhs)
{
	return (x * rhs.x + y * rhs.y + z * rhs.z);
}

template <typename TType>
IVector3<TType>	IVector3<TType>::cross(const IVector3<TType>& rhs)
{
	IVector3<TType>	res;

	res.x = y * rhs.z - z * rhs.y;
	res.y = z * rhs.x - x * rhs.z;
	res.z = x * rhs.y - y * rhs.x;

	return res;
}
