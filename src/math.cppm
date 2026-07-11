module;

#include <concepts>

export module math;

import types;

export template <std::floating_point T = f32>
struct Vec2 {
public:
    constexpr Vec2() : x(static_cast<T>(0)), y(static_cast<T>(0)) {}

    template <typename U> requires std::convertible_to<U, T>
    constexpr Vec2(U x, U y)
        : x(static_cast<T>(x)),
          y(static_cast<T>(y)) {}

    constexpr auto perp_dot(Vec2 const& rhs) -> f32 {
        return x * rhs.y - y * rhs.x;
    }
public:
    T x;
    T y;
};

export template <std::floating_point T = f32>
struct Vec3 {
public:
    constexpr Vec3()
        : x(static_cast<T>(0))
        , y(static_cast<T>(0))
        , z(static_cast<T>(0)) {}

    constexpr Vec3(T scalar)
        : x(scalar), y(scalar), z(scalar) {}

    constexpr Vec3(T vx, T vy = 0, T vz = 0)
        : x(vx), y(vy), z(vz) {}

    template <typename U> requires std::convertible_to<U, T>
    constexpr Vec3(Vec2<U> const& xy, T vz = 0)
        : x(static_cast<T>(xy.x))
        , y(static_cast<T>(xy.y))
        , z(vz) {}

    auto cross(Vec3 const& rhs) -> Vec3 {
        return Vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }
public:
    T x;
    T y;
    T z;
};

export template <std::floating_point T = f32>
constexpr auto operator-(Vec3<T> const& a, Vec3<T> const& b) -> Vec3<T> {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

export template <std::floating_point T = f32>
constexpr auto operator+(Vec3<T> const& a, Vec3<T> const& b) -> Vec3<T> {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

export using Color = Vec3<f32>;
export using Point = Vec3<f32>;
