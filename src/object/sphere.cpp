#include "sphere.hpp"
#include <cmath>

namespace {

inline void get_sphere_uv(const Vec3& unit_normal, float& u, float& v) {
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kInvPi = 1.0f / kPi;
    constexpr float kInvTwoPi = 1.0f / (2.0f * kPi);

    float theta = std::acos(-unit_normal.y);
    float phi = std::atan2(-unit_normal.z, unit_normal.x) + kPi;

    u = phi * kInvTwoPi;
    v = theta * kInvPi;
}

} // namespace

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
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.set_face_normal(ray, outward_normal);
    rec.material_id = material_id;
    rec.object_id = object_id;

    return true;
}

AABB Sphere::bounding_box() const {
    Vec3 radius_vec(radius, radius, radius);
    return AABB(center - radius_vec, center + radius_vec);
}