#pragma once

struct Vec4
{
	be_t<f32> X;
	be_t<f32> Y;
	be_t<f32> Z;
	be_t<f32> W;

	Vec4& add_vec4(Vec4& arg);
	Vec4& sub_vec4(Vec4& arg);
	Vec4& mul_i(f32 arg);
	void operator+=(Vec4& arg);
	be_t<f32> len();
	Vec4& div_i(f32 arg);
	Vec4& norm();
};
