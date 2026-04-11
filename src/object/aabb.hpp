#pragma once

#include "../math/ray.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

class AABB {
public:
    Point3 pmin;
    Point3 pmax;

    AABB()
        : pmin(std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()),
          pmax(-std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity(),
               -std::numeric_limits<float>::infinity()) {
    }

    AABB(const Point3& min_point, const Point3& max_point)
        : pmin(min_point), pmax(max_point) {
    }

    static AABB merge(const AABB& a, const AABB& b) {
        if (!a.is_valid()) {
            return b;
        }
        if (!b.is_valid()) {
            return a;
        }

        return AABB(
            Point3(std::min(a.pmin.x, b.pmin.x),
                   std::min(a.pmin.y, b.pmin.y),
                   std::min(a.pmin.z, b.pmin.z)),
            Point3(std::max(a.pmax.x, b.pmax.x),
                   std::max(a.pmax.y, b.pmax.y),
                   std::max(a.pmax.z, b.pmax.z))
        );
    }

    static AABB from_point(const Point3& p) {
        return AABB(p, p);
    }

    void expand(const Point3& p) {
        if (!is_valid()) {
            pmin = p;
            pmax = p;
            return;
        }

        pmin.x = std::min(pmin.x, p.x);
        pmin.y = std::min(pmin.y, p.y);
        pmin.z = std::min(pmin.z, p.z);
        pmax.x = std::max(pmax.x, p.x);
        pmax.y = std::max(pmax.y, p.y);
        pmax.z = std::max(pmax.z, p.z);
    }

    void expand(const AABB& b) {
        if (!b.is_valid()) {
            return;
        }

        if (!is_valid()) {
            *this = b;
            return;
        }

        pmin.x = std::min(pmin.x, b.pmin.x);
        pmin.y = std::min(pmin.y, b.pmin.y);
        pmin.z = std::min(pmin.z, b.pmin.z);
        pmax.x = std::max(pmax.x, b.pmax.x);
        pmax.y = std::max(pmax.y, b.pmax.y);
        pmax.z = std::max(pmax.z, b.pmax.z);
    }

    bool is_valid() const {
        return pmin.x <= pmax.x && pmin.y <= pmax.y && pmin.z <= pmax.z;
    }

    Vec3 extent() const {
        return pmax - pmin;
    }

    Point3 centroid() const {
        return 0.5f * (pmin + pmax);
    }

    float surface_area() const {
        if (!is_valid()) {
            return 0.0f;
        }

        Vec3 e = extent();
        return 2.0f * (e.x * e.y + e.x * e.z + e.y * e.z);
    }

    bool hit(const Ray& ray, float t_min, float t_max) const {
        if (!is_valid()) {
            return false;
        }

        constexpr float kParallelEpsilon = 1e-12f;

        const Vec3 origin = ray.getOrigin();
        const Vec3 direction = ray.getDirection();

        for (int axis = 0; axis < 3; ++axis) {
            const float o = origin[axis];
            const float d = direction[axis];
            const float min_v = pmin[axis];
            const float max_v = pmax[axis];

            if (std::abs(d) < kParallelEpsilon) {
                if (o < min_v || o > max_v) {
                    return false;
                }
                continue;
            }

            const float inv_d = 1.0f / d;
            float t0 = (min_v - o) * inv_d;
            float t1 = (max_v - o) * inv_d;
            if (t0 > t1) {
                std::swap(t0, t1);
            }

            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;

            // NaN-safe interval test: if either side is NaN this returns false.
            if (!(t_max >= t_min)) {
                return false;
            }
        }

        return true;
    }
};