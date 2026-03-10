#pragma once

#include "../math/vec3.hpp"
#include "../math/ray.hpp"


struct HitRecord {
    Point3 point;
    Vec3 normal;
    double t;
    int material_id;
};

class Object {
protected:
    int material_id;
public:
    Object(int mat_id) : material_id(mat_id) {}
    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual ~Object() = default;

    int getMaterialId() const { return material_id; }
};