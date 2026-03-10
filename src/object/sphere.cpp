#include "sphere.hpp"
#include <cmath>

bool Sphere::hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const {
    Vec3 oc = ray.getOrigin() - center;
    double c = oc.length_squared() - radius * radius;
    double half_b = dot(oc, ray.getDirection());
    if (c > 0.0 && half_b > 0.0) {
        return false;
    }

    double a = ray.getDirection().length_squared();
    double discriminant = half_b * half_b - a * c;

    if (discriminant < 0.0) {
        return false;
    }

    double sqrtd = std::sqrt(discriminant);

    double root = (-half_b - sqrtd) / a;
    if (root <= t_min || t_max <= root) {
        root = (-half_b + sqrtd) / a;
        if (root <= t_min || t_max <= root) {
            return false;
        }
    }

    rec.t = root;
    rec.point = ray.at(rec.t);
    rec.normal = (rec.point - center) / radius;
    rec.material_id = material_id;

    return true;
}