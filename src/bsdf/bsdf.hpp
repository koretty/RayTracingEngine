#pragma once

#include "../math/vec3.hpp"
#include "../object/object.hpp"
#include "../material/material.hpp"

struct BsdfSample {
    Vec3 wi{0.0f, 0.0f, 0.0f};
    Color weight{0.0f, 0.0f, 0.0f};
    float pdf{0.0f};
    bool valid{false};
    bool is_delta{false};
};

class IBSDF {
public:
    virtual ~IBSDF() = default;

    virtual BsdfSample sample(const Vec3& wo, const HitRecord& hit, const Material& material) const = 0;
    virtual Color eval(const Vec3& wo, const Vec3& wi, const HitRecord& hit, const Material& material) const = 0;
    virtual float pdf(const Vec3& wo, const Vec3& wi, const HitRecord& hit, const Material& material) const = 0;
};
