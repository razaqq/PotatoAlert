// Copyright 2021 <github.com/razaqq>
#pragma once

#include <cmath>


namespace PotatoAlert::Core {

struct Vec2
{
	float X, Y;

	inline static constexpr Vec2 Null()
	{
		return { 0.0f, 0.0f };
	}

	inline constexpr bool IsNull() const
	{
		return *this == Null();
	}

	inline constexpr Vec2 operator-() const
	{
		return { -X, -Y };
	}

	inline bool operator==(const Vec2 other) const
	{
		constexpr float delta = 0.00001;
		return std::abs(X - other.X) < delta && std::abs(Y - other.Y) < delta;
	}

	inline constexpr Vec2 operator+(const Vec2 other) const
	{
		return { X + other.X, Y + other.Y };
	}

	inline constexpr void operator+=(const Vec2 other)
	{
		X += other.X;
		Y += other.Y;
	}

	inline float Distance(Vec2 other) const
	{
		return std::sqrt((X - other.X) * (X - other.X) + (Y - other.Y) * (Y - other.Y));
	}
};

struct Vec3
{
	float X, Y, Z;

	inline static constexpr Vec3 Null()
	{
		return { 0.0f, 0.0f, 0.0f };
	}

	inline constexpr bool IsNull() const
	{
		return *this == Null();
	}

	inline constexpr Vec3 operator-() const
	{
		return { -X, -Y, -Z };
	}

	inline bool operator==(const Vec3 other) const
	{
		constexpr float delta = 0.00001;
		return std::abs(X - other.X) < delta && std::abs(Y - other.Y) < delta && std::abs(Z - other.Z) < delta;
	}

	inline constexpr Vec3 operator+(const Vec3 other) const
	{
		return { X + other.X, Y + other.Y, Z + other.Z };
	}

	inline constexpr void operator+=(const Vec3 other)
	{
		X += other.X;
		Y += other.Y;
		Z += other.Z;
	}
};

struct Vec4
{
	
};

struct Rot3
{
	float Yaw, Pitch, Roll;

	static constexpr Rot3 Null()
	{
		return { 0.0f, 0.0f, 0.0f };
	}
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
