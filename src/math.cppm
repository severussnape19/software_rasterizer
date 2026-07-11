module;

#include <cmath>
#include <concepts>
#include <cassert>

export module math;

import types;

export template <std::floating_point T = f32>
struct Vec2 {
public:
    constexpr Vec2() noexcept : x(static_cast<T>(0)), y(static_cast<T>(0)) {}

    template <typename U> requires std::convertible_to<U, T>
    constexpr Vec2(U x, U y) noexcept
        : x(static_cast<T>(x)),
          y(static_cast<T>(y)) {}

    constexpr auto perp_dot(Vec2 const& rhs) const noexcept -> T {
        return x * rhs.y - y * rhs.x;
    }
public:
    T x;
    T y;
};

export template <std::floating_point T = f32>
struct Vec3 {
public:
    constexpr Vec3() noexcept
        : x(static_cast<T>(0))
        , y(static_cast<T>(0))
        , z(static_cast<T>(0)) {}

    constexpr Vec3(T scalar) noexcept
        : x(scalar), y(scalar), z(scalar) {}

    constexpr Vec3(T vx, T vy = 0, T vz = 0) noexcept
        : x(vx), y(vy), z(vz) {}

    template <typename U> requires std::convertible_to<U, T>
    constexpr Vec3(Vec2<U> const& xy, T vz = 0) noexcept
        : x(static_cast<T>(xy.x))
        , y(static_cast<T>(xy.y))
        , z(vz) {}

    constexpr auto operator/(T scalar) const noexcept -> Vec3 {
        assert(scalar != static_cast<T>(0));
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    constexpr auto operator*(T scalar) const noexcept -> Vec3 {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    constexpr auto cross(Vec3 const& rhs) const noexcept -> Vec3 {
        return Vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    constexpr auto dot(Vec3 const& rhs) const noexcept -> T {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    constexpr auto length_sq() const noexcept -> T {
        return x*x + y*y + z*z;
    }

    constexpr auto length() const noexcept -> f32 {
        auto length_squared = this->length_sq();
        assert(length_squared != static_cast<T>(0));
        return std::sqrt(length_squared);
    }

    constexpr auto normalized() const noexcept -> Vec3 {
        auto len = this->length();
        assert(len != static_cast<T>(0));

        T inv_len = static_cast<T>(1) / len;
        return *this * inv_len;
    }

public:
    T x;
    T y;
    T z;
};

export template <std::floating_point T = f32>
constexpr auto operator-(Vec3<T> const& a, Vec3<T> const& b) noexcept -> Vec3<T> {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

export template <std::floating_point T = f32>
constexpr auto operator+(Vec3<T> const& a, Vec3<T> const& b) noexcept -> Vec3<T> {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

export template <std::floating_point T = f32>
constexpr auto dot(Vec3<T> const& a, Vec3<T> const& b) noexcept -> T {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

export using Color = Vec3<f32>;
export using Point = Vec3<f32>;
