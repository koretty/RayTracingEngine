#pragma once
#include <random>

inline float random_float() {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

inline float random_float(float min, float max) {
    return min + (max - min) * random_float();
}

#include "vec3.hpp"

inline Vec3 random_vec() {
    return Vec3(random_float(), random_float(), random_float());
}

inline Vec3 random_vec(float min, float max) {
    return Vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

inline Vec3 random_in_unit_sphere() {
    while (true) {
        auto p = random_vec(-1.0f, 1.0f);
        if (p.length_squared() >= 1.0f) continue;
        return p;
    }
}

inline Vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}