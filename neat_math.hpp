#ifndef NEAT_MATH_HPP_
#define NEAT_MATH_HPP_

#include <cmath>
#include <initializer_list>
#include <iterator>

namespace neat::math {

template <typename T> const T pi = static_cast<T>(3.14159265359);

template <typename T> constexpr T approach(T value, T target, T max);
template <typename T> constexpr T abs(T value);
template <typename T> constexpr T sign(T value);
template <typename T> constexpr T clamp(T value, T min, T max);
template <typename T> constexpr T clip(T value, T min, T max);
template <typename T> constexpr T lerp(T value, T start, T end);
template <typename T> constexpr T normalize(T value, T start, T end);
template <typename T> constexpr T rescale(T value, T oldMin, T oldMax, T newMin, T newMax);
template <typename T> constexpr T absdiff(T a, T b);

inline constexpr size_t           factorial(size_t n);
inline constexpr size_t           binomial_coefficient(size_t n, size_t k);
template <typename T> constexpr T polynomial(T t, std::initializer_list<T> coefficients);

template <typename T, typename Eps> constexpr bool equals(T a, T b, Eps epsilon = 1e-6);

template <typename T> constexpr bool within(T value, T start, T end);
template <typename T> constexpr bool overlap(T min0, T max0, T min1, T max1);

template <typename T> struct point { T x, y; };
template <typename T> struct circle { T x, y, radius; };
template <typename T> struct rectangle { T x, y, width, height; };

template <typename T> constexpr T length(const point<T>& p);
template <typename T> constexpr T length2(const point<T>& p);

template <typename T> constexpr bool collide(const point<T>& p, const rectangle<T>& r);
template <typename T> constexpr bool collide(const point<T>& p, const circle<T>& c);

template <typename T> constexpr bool collide(const circle<T>& c, const point<T> p);
template <typename T> constexpr bool collide(const circle<T>& a, const circle<T> b);
template <typename T> constexpr bool collide(const circle<T>& c, const rectangle<T> r);

template <typename T> constexpr bool collide(const rectangle<T>& r, const point<T>& p);
template <typename T> constexpr bool collide(const rectangle<T>& r, const circle<T> c);
template <typename T> constexpr bool collide(const rectangle<T>& a, const rectangle<T> b);

namespace smoothstep {

template <typename T> constexpr T cosine(T t);
template <typename T> constexpr T linear(T t);
template <typename T> constexpr T cubic(T t);
template <typename T> constexpr T quintic(T t);

namespace inverse {

template <typename T> constexpr T cosine(T t);
template <typename T> constexpr T linear(T t);
template <typename T> constexpr T cubic(T t);

}  // namespace inverse

};  // namespace smoothstep

namespace angle {

template <typename T> constexpr T to_radians(T degrees);
template <typename T> constexpr T to_degrees(T radians);

}  // namespace angle

}  // namespace neat::math

#pragma region implementations

template <typename T> constexpr T neat::math::approach(T value, T target, T max) {
    max = abs(max);
    if (absdiff(value, target) < max)
        return target;

    if (value < target)
        return value + max;
    else
        return value - max;
}

template <typename T> constexpr T neat::math::abs(T value) {
    if (value < 0)
        return -value;
    else
        return value;
}

template <typename T> constexpr T neat::math::sign(T value) {
    if (value == static_cast<T>(0))
        return static_cast<T>(0);
    else
        return value / abs(value);
}

template <typename T> constexpr T neat::math::clamp(T value, T min, T max) {
    if (max < min)
        return clamp(value, max, min);
    else if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

template <typename T> constexpr T neat::math::clip(T value, T min, T max) {
    return clamp(value, min, max);
}

template <typename T> constexpr T neat::math::lerp(T value, T start, T end) {
    return start + value * (end - start);
}

template <typename T> constexpr T neat::math::normalize(T value, T start, T end) {
    return (value - start) / (end - start);
}

template <typename T> constexpr T neat::math::rescale(T value, T oldMin, T oldMax, T newMin, T newMax) {
    return normalize(value, oldMin, oldMax) * (newMax - newMin) + newMin;
}

template <typename T> constexpr T neat::math::absdiff(T a, T b) {
    if (a > b)
        return a - b;
    else
        return b - a;
}

inline constexpr size_t neat::math::factorial(size_t n) {
    size_t result = 1;
    for (size_t i = 2; i <= n; i++)
        result *= i;
    return result;
}

inline constexpr size_t neat::math::binomial_coefficient(size_t n, size_t k) {
    return factorial(n) / (factorial(k) * factorial(n - k));
}

template <typename T> constexpr T neat::math::polynomial(T t, std::initializer_list<T> coefficients) {
    T result = static_cast<T>(0);
    T tcurr  = static_cast<T>(1);

    for (auto it = std::rbegin(coefficients); it != std::rend(coefficients); ++it) {
        T coeff = *it;
        result += tcurr * coeff;
        tcurr *= t;
    }

    return result;
}

template <typename T, typename Eps> constexpr bool neat::math::equals(T a, T b, Eps epsilon) {
    return absdiff(a, b) <= static_cast<T>(epsilon);
}

template <typename T> constexpr bool neat::math::within(T value, T start, T end) {
    if (end < start)
        return within(value, end, start);
    else
        return (start <= value) && (value <= end);
}

template <typename T> constexpr bool neat::math::overlap(T min0, T max0, T min1, T max1) {
    if (max0 < min0)
        return overlap(max0, min0, min1, max1);
    else if (max1 < min1)
        return overlap(min0, max0, max1, min1);
    else
        return !((max0 < min1) || (max1 < min0));
}

template <typename T> constexpr T neat::math::length(const point<T>& p) {
    return std::sqrt(length2(p));
}

template <typename T> constexpr T neat::math::length2(const point<T>& p) {
    return p.x * p.x + p.y * p.y;
}

template <typename T> constexpr bool neat::math::collide(const point<T>& p, const rectangle<T>& r) {
    return (r.x < p.x) && (p.x < r.x + r.width) && (r.y < p.y) && (p.y < r.x + r.height);
}

template <typename T> constexpr bool neat::math::collide(const point<T>& p, const circle<T>& c) {
    return length2(point<T> {absdiff(p.x, c.x), absdiff(p.y, c.y)}) < c.radius * c.radius;
}

template <typename T> constexpr bool neat::math::collide(const circle<T>& c, const point<T> p) {
    return collide(p, c);
}

template <typename T> constexpr bool neat::math::collide(const circle<T>& a, const circle<T> b) {
    T dx   = absdiff(a.x, b.x);
    T dy   = absdiff(a.y, a.y);
    T rsum = a.radius + b.radius;
    return dx * dx + dy * dy < rsum * rsum;
}

template <typename T> constexpr bool neat::math::collide(const circle<T>& c, const rectangle<T> r) {
    // based on https://www.jeffreythompson.org/collision-detection/circle-rect.php
    float testx = c.x;
    float testy = c.y;

    if (c.x < r.x)
        testx = r.x;
    else if (c.x > r.x + r.width)
        testx = r.x + r.width;
    if (c.y < r.y)
        testy = r.y;
    else if (c.y > r.y + r.height)
        testy = r.y + r.height;

    float dx = absdiff(c.x, testx);
    float dy = absdiff(c.y, testy);

    return dx * dx + dy * dy < c.radius * c.radius;
}

template <typename T> constexpr bool neat::math::collide(const rectangle<T>& r, const point<T>& p) {
    return collide(p, r);
}

template <typename T> constexpr bool neat::math::collide(const rectangle<T>& r, const circle<T> c) {
    return collide(c, r);
}

template <typename T> constexpr bool neat::math::collide(const rectangle<T>& a, const rectangle<T> b) {
    return (a.x < b.x + b.width &&
            a.y < b.y + b.height &&
            a.x + a.width > b.x &&
            a.y + a.height > b.y);
}

template <typename T> constexpr T neat::math::smoothstep::cosine(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return static_cast<T>(0.5) - std::cos(pi<T> * t) * static_cast<T>(0.5);
}

template <typename T> constexpr T neat::math::smoothstep::linear(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return t;
}

template <typename T> constexpr T neat::math::smoothstep::cubic(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return t * t * (3 - 2 * t);
}
template <typename T> constexpr T neat::math::smoothstep::quintic(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return t * t * t * (10 - 15 * t + 6 * t * t);
}

template <typename T> constexpr T neat::math::smoothstep::inverse::cosine(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return std::acos(static_cast<T>(1) - static_cast<T>(2) * t) / pi<T>;
}

template <typename T> constexpr T neat::math::smoothstep::inverse::linear(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return t;
}

template <typename T> constexpr T neat::math::smoothstep::inverse::cubic(T t) {
    t = clamp(t, static_cast<T>(0), static_cast<T>(1));
    return static_cast<T>(0.5) - std::sin(std::asin(static_cast<T>(1.0) - static_cast<T>(2.0) * t) / static_cast<T>(3.0));
}

template <typename T> constexpr T neat::math::angle::to_radians(T degrees) {
    return static_cast<T>(degrees * pi<T> / 180.0);
}

template <typename T> constexpr T neat::math::angle::to_degrees(T radians) {
    return static_cast<T>(radians * 180.0 / pi<T>);
}

#pragma endregion implementations

#endif  // NEAT_MATH_HPP_