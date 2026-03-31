#include "sphere.hpp"
#include <cmath>

bool Sphere::hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Vec3 oc = ray.getOrigin() - center;
    float c = oc.length_squared() - radius * radius;
    float half_b = dot(oc, ray.getDirection());
    if (c > 0.0f && half_b > 0.0f) {
        return false;
    }

    float a = ray.getDirection().length_squared();
    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0.0f) {
        return false;
    }

    float sqrtd = std::sqrt(discriminant);

    float root = (-half_b - sqrtd) / a;
    if (root <= t_min || t_max <= root) {
        root = (-half_b + sqrtd) / a;
        if (root <= t_min || t_max <= root) {
            return false;
        }
    }

    rec.t = root;
    rec.point = ray.at(rec.t);
    Vec3 outward_normal = (rec.point - center) / radius;
    rec.set_face_normal(ray, outward_normal);
    rec.material_id = material_id;

    return true;
}