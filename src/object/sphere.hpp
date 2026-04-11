#pragma once

#include "object.hpp"

class Sphere : public Object {
    Point3 center;
    float radius;
public:
    Sphere(Point3 cen, float r, int material_id) : Object(material_id), center(cen), radius(r) {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    virtual AABB bounding_box() const override;

    Point3 getCenter() const { return center; }
    float getRadius() const { return radius; }
};
