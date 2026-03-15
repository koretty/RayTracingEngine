#pragma once

#include "../math/ray.hpp"
#include "../math/math_utils.hpp"
#include <cmath>
#include <numbers>

class Camera {
    Point3 origin;
    Point3 lookat;
    Vec3 up;
    Vec3 front;
    Vec3 right;
    Point3 lower_left_corner;
    float viewport_h;
    float viewport_v;
    float fov_deg;
    float aspect_ratio;
    float aperture;
    float lens_radius;
    float focus_dist;

    static Vec3 random_in_unit_disk() {
        while (true) {
            Vec3 p(random_float(-1.0f, 1.0f), random_float(-1.0f, 1.0f), 0.0f);
            if (p.length_squared() >= 1.0f) continue;
            return p;
        }
    }
public:
    Camera(
        const Point3& origin,
        const Point3& lookat,
        const Vec3& view_up,
        float fov_deg,
        float aspect_ratio,
        float aperture,
        float focus_dist
    ):
    origin(origin), 
    lookat(lookat),  
    fov_deg(fov_deg), 
    aspect_ratio(aspect_ratio), 
    aperture(aperture), 
    lens_radius(aperture * 0.5f),
    focus_dist(focus_dist){
        front = unit_vector(lookat - origin);
        right = unit_vector(cross(front, view_up));
        up = cross(right, front);
        float fov_rad = fov_deg * static_cast<float>(std::numbers::pi) / 180.0f;
        viewport_v = 2.0f * tan(fov_rad * 0.5f) * focus_dist;
        viewport_h = aspect_ratio * viewport_v;
        lower_left_corner = origin - viewport_h * 0.5f * right - viewport_v * 0.5f * up  + front * focus_dist;
    }

    Ray get_ray(float u, float v) const {
        Vec3 rd = lens_radius * random_in_unit_disk();
        Vec3 offset = right * rd.x + up * rd.y;

        Point3 ray_origin = origin + offset;
        Point3 pixel_on_focus_plane = lower_left_corner + u * viewport_h * right + v * viewport_v * up;
        return Ray(ray_origin, pixel_on_focus_plane - ray_origin);
    }
};