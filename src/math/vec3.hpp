#pragma once

#include <cmath>

class Vec3{
public:

    float x;
    float y;
    float z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    float operator[](int i) const {
        return i == 0 ? x : (i == 1 ? y : z);
    }
    float& operator[](int i) {
        return i == 0 ? x : (i == 1 ? y : z);
    }

    Vec3& operator+=(const Vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3& operator*=(float t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3& operator/=(float t) {
        return *this *= 1.0f / t;
    }

    float length_squared() const {
        return x*x + y*y + z*z;
    }

    float length() const {
        return std::sqrt(length_squared());
    }

    bool near_zero() const {
        const auto s = 1e-8f;
        return (std::abs(x) < s) && (std::abs(y) < s) && (std::abs(z) < s);
    }
};

inline Vec3 operator+(const Vec3& u, const Vec3& v) {
    return Vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline Vec3 operator-(const Vec3& u, const Vec3& v) {
    return Vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline Vec3 operator*(const Vec3& u, const Vec3& v) {
    return Vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline Vec3 operator*(float t, const Vec3& v) {
    return Vec3(t * v.x, t * v.y, t * v.z);
}

inline Vec3 operator*(const Vec3& v, float t) {
    return t * v;
}

inline Vec3 operator/(Vec3 v, float t) {
    return (1.0f / t) * v;
}

inline float dot(const Vec3& u, const Vec3& v) {
    return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}

inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0f * dot(v, n) * n;
}

inline Vec3 refract(const Vec3& uv, const Vec3& n, float etai_over_etat) {
    float cos_theta = std::fmin(dot(-uv, n), 1.0f);
    Vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

using Point3 = Vec3;   
using Color = Vec3;   