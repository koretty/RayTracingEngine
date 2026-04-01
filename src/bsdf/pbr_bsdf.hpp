#pragma once

#include "bsdf.hpp"

class PbrBsdf final : public IBSDF {
public:
    BsdfSample sample(const Vec3& wo, const HitRecord& hit, const Material& material) const override;
    Color eval(const Vec3& wo, const Vec3& wi, const HitRecord& hit, const Material& material) const override;
    float pdf(const Vec3& wo, const Vec3& wi, const HitRecord& hit, const Material& material) const override;
};
