#pragma once

#include "../math/vec3.hpp"
#include "../math/ray.hpp"
#include "aabb.hpp"


struct HitRecord {
    Point3 point;
    Vec3 normal;
    float u{0.0f};
    float v{0.0f};
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
    int object_id{-1};
public:
    explicit Object(int mat_id = -1) : material_id(mat_id) {}
    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
    virtual AABB bounding_box() const = 0;
    virtual ~Object() = default;

    int getMaterialId() const { return material_id; }
    void set_object_id(int id) { object_id = id; }
    int get_object_id() const { return object_id; }
};