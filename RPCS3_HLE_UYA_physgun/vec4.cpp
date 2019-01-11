#include "stdafx.h"
#include "RPCS3/Utilities/BEType.h"
#include "vec4.h"

Vec4& Vec4::add_vec4(Vec4& arg)
{
	Vec4 res{};
	res.X = X + arg.X;
	res.Y = Y + arg.Y;
	res.Z = Z + arg.Z;
	res.W = 1.0f;
	return res;
}

Vec4& Vec4::sub_vec4(Vec4& arg)
{
	Vec4 res{};
	res.X = X - arg.X;
	res.Y = Y - arg.Y;
	res.Z = Z - arg.Z;
	res.W = 1.0f;
	return res;
}

Vec4& Vec4::mul_i(f32 arg)
{
	Vec4 res{};
	res.X = X * arg;
	res.Y = Y * arg;
	res.Z = Z * arg;
	res.W = 1.0f;
	return res;
}

void Vec4::operator+=(Vec4& arg)
{
	X += arg.X;
	Y += arg.Y;
	Z += arg.Z;
}

be_t<f32> Vec4::len()
{
	return sqrt(X * X + Y * Y + Z * Z);
}

Vec4& Vec4::div_i(f32 arg)
{
	Vec4 res{};
	res.X = X / arg;
	res.Y = Y / arg;
	res.Z = Z / arg;
	res.W = 1;
	return res;
}

Vec4& Vec4::norm()
{
	Vec4 res{};
	const auto len = Vec4::len();
	res.X = X / len;
	res.Y = Y / len;
	res.Z = Z / len;
	res.W = 1.0;
	return res;
}
