#pragma once

#include "material.hpp"
#include <algorithm>
#include <cmath>

namespace material_optics {

inline Color beer_lambert_transmittance(const Material& material, float distance) {
    float d = std::max(0.0f, distance);
    const Color& sigma = material.absorption_coefficient;

    return Color(
        std::exp(-std::max(0.0f, sigma.x) * d),
        std::exp(-std::max(0.0f, sigma.y) * d),
        std::exp(-std::max(0.0f, sigma.z) * d)
    );
}

} // namespace material_optics
