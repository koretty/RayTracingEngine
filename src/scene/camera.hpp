#pragma once

#include "../math/ray.hpp"
#include <cmath>
#include <numbers>

class Camera {
    Point3 origin;
    Point3 lookat;
    Vec3 up;
    Vec3 front;
    Vec3 right;
    Point3 lower_left_corner;
    double viewport_h;
    double viewport_v;
    double fov_deg;
    double aspect_ratio;
    double aperture;
    double focus_dist;
public:
    Camera(
        const Point3& origin,
        const Point3& lookat,
        const Vec3& view_up,
        double fov_deg,
        double aspect_ratio,
        double aperture,
        double focus_dist
    ):
    origin(origin), 
    lookat(lookat),  
    fov_deg(fov_deg), 
    aspect_ratio(aspect_ratio), 
    aperture(aperture), 
    focus_dist(focus_dist){
        front = unit_vector(lookat - origin);
        right = unit_vector(cross(front, view_up));
        up = cross(right, front);
        double fov_rad = fov_deg * std::numbers::pi / 180.0;
        viewport_v = 2.0 * tan(fov_rad * 0.5) * focus_dist;
        viewport_h = aspect_ratio * viewport_v;
        lower_left_corner = origin - viewport_h * 0.5 * right - viewport_v * 0.5 * up  + front * focus_dist;
    }

    Ray get_ray(double u, double v) const {
        return Ray(origin, lower_left_corner + u * viewport_h * right + v * viewport_v * up - origin);
    }
};