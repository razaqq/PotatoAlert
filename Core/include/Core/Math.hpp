// Copyright 2021 <github.com/razaqq>
#pragma once


namespace PotatoAlert::Core {

struct Vec2
{
	float X, Y;
};

struct Vec3
{
	float X, Y, Z;
};

struct Vec4
{
	
};

struct Rot3
{
	float Yaw, Pitch, Roll;
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

}  // namespace PotatoAlert::Core
