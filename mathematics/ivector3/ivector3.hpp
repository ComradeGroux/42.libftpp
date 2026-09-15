#pragma once

template <typename TType>
struct IVector3
{
	IVector3(void);
	IVector3(TType x, TType y, TType z);

	TType	x;
	TType	y;
	TType	z;

	IVector3<TType>		operator+(const IVector3<TType>& rhs);
	IVector3<TType>&	operator+=(const IVector3<TType>& rhs);
	IVector3<TType>		operator-(const IVector3<TType>& rhs);
	IVector3<TType>&	operator-=(const IVector3<TType>& rhs);
	IVector3<TType>		operator*(const IVector3<TType>& rhs);
	IVector3<TType>&	operator*=(const IVector3<TType>& rhs);
	IVector3<TType>		operator/(const IVector3<TType>& rhs);
	IVector3<TType>&	operator/=(const IVector3<TType>& rhs);

	bool	operator==(const IVector3<TType>& rhs);
	bool	operator!=(const IVector3<TType>& rhs);

	float			length(void);
	IVector3<float>	normalize(void);
	float			dot(const IVector3<TType>& rhs);
	IVector3<TType>	cross(const IVector3<TType>& rhs);
};

#include "ivector3.tpp"
