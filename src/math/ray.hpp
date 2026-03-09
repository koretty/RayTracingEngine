#pragma once

#include "vec3.hpp"

class Ray {
    Vec3 origin;    
    Vec3 direction;  
public:
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

    Vec3 getOrigin() const { return origin; }
    Vec3 getDirection() const { return direction; }
    Vec3 at(double t) const {return origin + t * direction;}
};
