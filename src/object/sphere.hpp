#pragma once

#include "object.hpp"

class Sphere : public Object {
    Point3 center;
    double radius;
public:
    Sphere(Point3 cen, double r, int material_id) : center(cen), radius(r), Object(material_id) {}

    virtual bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override;

    Point3 getCenter() const { return center; }
    double getRadius() const { return radius; }
};
