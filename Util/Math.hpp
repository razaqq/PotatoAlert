// Copyright 2021 <github.com/razaqq>
#pragma once

namespace PotatoAlert
{

struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;
};

struct Vec4
{
	
};

struct Rot3
{
	float roll, pitch, yaw;
};

struct Mat3
{
	
};

struct Mat4
{
	float m_data[4][4];

	float& operator()(size_t i, size_t j)
	{
		return m_data[i][j];
	}

	const float& operator()(size_t i, size_t j) const
	{
		return m_data[i][j];
	}
};

}  // namespace PotatoAlert
