#pragma once

#include "../math/vec3.hpp"
#include "../math/ray.hpp"


struct HitRecord {
    Point3 point;
    Vec3 normal;
    float t;
    int material_id;
    int object_id{-1};
    bool front_face;

    void set_face_normal(const Ray& ray, const Vec3& outward_normal) {
        front_face = dot(ray.getDirection(), outward_normal) < 0.0f;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Object {
protected:
    int material_id;
public:
    Object(int mat_id) : material_id(mat_id) {}
    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
    virtual ~Object() = default;

    int getMaterialId() const { return material_id; }
};