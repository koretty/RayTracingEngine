#pragma once

#include "../math/vec3.hpp"

struct Material {
    Color base_color{1.0f, 1.0f, 1.0f};
    float metallic{0.0f};
    float roughness{0.5f};
    float transmission{0.0f};
    float ior{1.5f};
    Color absorption_coefficient{0.0f, 0.0f, 0.0f};
    Color emission{0.0f, 0.0f, 0.0f};
};