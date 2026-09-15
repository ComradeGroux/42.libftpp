#pragma once

template <typename TType>
struct IVector2
{
	IVector2(void);
	IVector2(TType x, TType y);

	TType	x;
	TType	y;

	IVector2<TType>		operator+(const IVector2<TType>& rhs);
	IVector2<TType>&	operator+=(const IVector2<TType>& rhs);
	IVector2<TType>		operator-(const IVector2<TType>& rhs);
	IVector2<TType>&	operator-=(const IVector2<TType>& rhs);
	IVector2<TType>		operator*(const IVector2<TType>& rhs);
	IVector2<TType>&	operator*=(const IVector2<TType>& rhs);
	IVector2<TType>		operator/(const IVector2<TType>& rhs);
	IVector2<TType>&	operator/=(const IVector2<TType>& rhs);

	bool	operator==(const IVector2<TType>& rhs);
	bool	operator!=(const IVector2<TType>& rhs);

	float			length(void) const;
	IVector2<float>	normalize(void) const;
	float			dot(const IVector2<TType>& rhs) const noexcept;
	IVector2<TType>	perp(void) const noexcept;
};

#include "ivector2.tpp"
