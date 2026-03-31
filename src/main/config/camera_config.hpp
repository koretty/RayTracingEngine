#pragma once

#include "../../scene/camera.hpp"

namespace config {
namespace camera {

inline Point3 origin = Point3(0.0f, 1.2f, 3.5f);
inline Point3 lookat = Point3(0.0f, 0.5f, 0.0f);
inline Vec3 vup = Vec3(0.0f, 1.0f, 0.0f);
inline float fov = 35.0f;
inline float aperture = 0.0f;
inline float move_speed = 0.5f;

inline Camera make_camera(int width, int height) {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float focus_dist = (origin - Point3(0.0f, 0.5f, 0.0f)).length();
    return Camera(origin, lookat, vup, fov, aspect, aperture, focus_dist);
}

} 
} 
