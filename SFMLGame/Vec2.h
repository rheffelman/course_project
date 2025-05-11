#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cassert>

template <typename T>
class Vec2
{
public:
	T x = 0;
	T y = 0;

	Vec2() = default;

	Vec2(T x, T y)
		:x(x), y(y)
	{ }

	//Constructor to convert from sf::Vector2
	Vec2(const sf::Vector2<T>& vec)
		:x(vec.x), y(vec.y)
	{ }

	//Allow aytinatic conversion to sf::Vector2
	//This lets us pass Vec2 into sfml functions
	operator sf::Vector2<T>()
	{
		return sf::Vector2<T>(x, y);
	}

	Vec2 operator+ (const Vec2& rhs) const
	{
		return Vec2(x +rhs.x, y + rhs.y);
	}

	Vec2 operator- (const Vec2& rhs) const
	{
		return Vec2(x - rhs.x, y - rhs.y);
	}

	Vec2 operator/ (const T val) const
	{
		assert(val != 0);
		return Vec2(x / val, y / val);
	}

	Vec2 operator* (const T val) const
	{
		return Vec2(x * val, y * val );
	}

	bool operator== (const Vec2& rhs) const
	{
		return (x == rhs.x and y == rhs.y);
	}

	bool operator!= (const Vec2& rhs) const
	{
		return !(x == rhs.x and y == rhs.y);
	}

	void operator+= (const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
	}

	void operator-= (const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
	}

	void operator*= (const T val)
	{
		x *= val;
		y *= val;
	}

	void operator/= (const T val)
	{
		assert(val != 0);
		x /= val;
		y /= val;
	}

	float dist(const Vec2& rhs) const
	{
		return abs(sqrt((rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y)));
	}
};

using Vec2f = Vec2<float>;