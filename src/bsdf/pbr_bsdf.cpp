#include "pbr_bsdf.hpp"
#include "../math/math_utils.hpp"
#include <algorithm>
#include <cmath>

namespace {

constexpr float kEpsilon = 1e-6f;

struct SamplingProbabilities {
    float diffuse{0.0f};
    float specular{0.0f};
    float transmission{0.0f};
};

inline float clamp01(float v) {
    return std::clamp(v, 0.0f, 1.0f);
}

inline float luminance(const Color& c) {
    return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
}

inline Color clamp_color01(const Color& c) {
    return Color(clamp01(c.x), clamp01(c.y), clamp01(c.z));
}

inline Vec3 normalize_or(const Vec3& v, const Vec3& fallback) {
    if (v.near_zero()) {
        return fallback;
    }
    return unit_vector(v);
}

inline float dielectric_f0(float ior) {
    float clamped_ior = std::max(ior, 1.0001f);
    float r0 = (1.0f - clamped_ior) / (1.0f + clamped_ior);
    return r0 * r0;
}

inline float schlick_fresnel_scalar(float cos_theta, float f0) {
    float t = 1.0f - clamp01(cos_theta);
    float t2 = t * t;
    float t5 = t2 * t2 * t;
    return f0 + (1.0f - f0) * t5;
}

inline Color schlick_fresnel(float cos_theta, const Color& f0) {
    float t = 1.0f - clamp01(cos_theta);
    float t2 = t * t;
    float t5 = t2 * t2 * t;
    return f0 + (Color(1.0f, 1.0f, 1.0f) - f0) * t5;
}

inline Color compute_f0(const Material& material) {
    float metallic = clamp01(material.metallic);
    float f0_scalar = dielectric_f0(material.ior);
    Color dielectric = Color(f0_scalar, f0_scalar, f0_scalar);
    Color base = clamp_color01(material.base_color);
    return dielectric * (1.0f - metallic) + base * metallic;
}

inline float ggx_distribution(float n_dot_h, float alpha) {
    float a2 = alpha * alpha;
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denom = n_dot_h2 * (a2 - 1.0f) + 1.0f;
    return a2 / (3.14159265358979323846f * denom * denom + kEpsilon);
}

inline float schlick_ggx_geometry_term(float n_dot_x, float alpha) {
    float k = ((alpha + 1.0f) * (alpha + 1.0f)) * 0.125f;
    return n_dot_x / (n_dot_x * (1.0f - k) + k + kEpsilon);
}

inline float smith_geometry(float n_dot_v, float n_dot_l, float alpha) {
    return schlick_ggx_geometry_term(n_dot_v, alpha) * schlick_ggx_geometry_term(n_dot_l, alpha);
}

inline void build_orthonormal_basis(const Vec3& n, Vec3& tangent, Vec3& bitangent) {
    if (std::abs(n.z) < 0.999f) {
        tangent = unit_vector(cross(Vec3(0.0f, 0.0f, 1.0f), n));
    } else {
        tangent = unit_vector(cross(Vec3(0.0f, 1.0f, 0.0f), n));
    }
    bitangent = cross(n, tangent);
}

inline Vec3 to_world(const Vec3& local_dir, const Vec3& n) {
    Vec3 tangent;
    Vec3 bitangent;
    build_orthonormal_basis(n, tangent, bitangent);
    return local_dir.x * tangent + local_dir.y * bitangent + local_dir.z * n;
}

inline Vec3 sample_cosine_hemisphere(const Vec3& n) {
    float u1 = random_float();
    float u2 = random_float();

    float r = std::sqrt(u1);
    float phi = 2.0f * 3.14159265358979323846f * u2;
    float x = r * std::cos(phi);
    float y = r * std::sin(phi);
    float z = std::sqrt(std::max(0.0f, 1.0f - u1));

    Vec3 local_dir(x, y, z);
    return normalize_or(to_world(local_dir, n), n);
}

inline Vec3 sample_ggx_half_vector(const Vec3& n, float alpha) {
    float u1 = random_float();
    float u2 = random_float();

    float phi = 2.0f * 3.14159265358979323846f * u1;
    float alpha2 = alpha * alpha;
    float cos_theta = std::sqrt((1.0f - u2) / (1.0f + (alpha2 - 1.0f) * u2));
    float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));

    Vec3 h_local(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
    return normalize_or(to_world(h_local, n), n);
}

inline SamplingProbabilities compute_sampling_probabilities(const Material& material) {
    SamplingProbabilities p;

    float transmission = clamp01(material.transmission);
    float reflection_budget = 1.0f - transmission;

    Color base = clamp_color01(material.base_color);
    Color f0 = compute_f0(material);

    float diffuse_energy = std::max(0.0f, (1.0f - clamp01(material.metallic)) * luminance(base));
    float specular_energy = std::max(0.01f, luminance(f0));

    float reflection_sum = diffuse_energy + specular_energy;
    float diffuse_ratio = 0.0f;
    float specular_ratio = 1.0f;

    if (reflection_sum > kEpsilon) {
        diffuse_ratio = diffuse_energy / reflection_sum;
        specular_ratio = specular_energy / reflection_sum;
    }

    p.transmission = transmission;
    p.diffuse = reflection_budget * diffuse_ratio;
    p.specular = reflection_budget * specular_ratio;
    return p;
}

inline float ggx_reflection_pdf(const Vec3& wo, const Vec3& wi, const Vec3& n, float alpha) {
    float n_dot_o = dot(n, wo);
    float n_dot_i = dot(n, wi);
    if (n_dot_o <= 0.0f || n_dot_i <= 0.0f) {
        return 0.0f;
    }

    Vec3 h = wo + wi;
    if (h.near_zero()) {
        return 0.0f;
    }
    h = unit_vector(h);

    float n_dot_h = std::max(0.0f, dot(n, h));
    float o_dot_h = std::max(kEpsilon, std::abs(dot(wo, h)));
    float d = ggx_distribution(n_dot_h, alpha);
    float pdf_h = d * n_dot_h;
    return pdf_h / (4.0f * o_dot_h + kEpsilon);
}

} // namespace

BsdfSample PbrBsdf::sample(const Vec3& wo_in, const HitRecord& hit, const Material& material) const {
    BsdfSample result;

    Vec3 n = normalize_or(hit.normal, Vec3(0.0f, 1.0f, 0.0f));
    Vec3 wo = normalize_or(wo_in, n);

    SamplingProbabilities p = compute_sampling_probabilities(material);
    float event = random_float();

    if (event < p.transmission && p.transmission > 0.0f) {
        float clamped_ior = std::max(material.ior, 1.0001f);
        float eta = hit.front_face ? (1.0f / clamped_ior) : clamped_ior;

        float cos_theta = std::min(dot(wo, n), 1.0f);
        float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
        bool cannot_refract = eta * sin_theta > 1.0f;

        float f0 = dielectric_f0(clamped_ior);
        float fresnel = schlick_fresnel_scalar(cos_theta, f0);

        Vec3 wi;
        if (cannot_refract || random_float() < fresnel) {
            wi = reflect(-wo, n);
            result.weight = Color(1.0f, 1.0f, 1.0f);
        } else {
            wi = refract(-wo, n, eta);
            result.weight = clamp_color01(material.base_color) * (1.0f - fresnel);
        }

        float roughness = clamp01(material.roughness);
        if (roughness > 0.001f) {
            wi = normalize_or(wi + (roughness * roughness * 0.25f) * random_in_unit_sphere(), wi);
        }

        result.wi = wi;
        result.pdf = p.transmission;
        result.is_delta = true;
        result.valid = true;
        result.weight = result.weight * (p.transmission / std::max(result.pdf, kEpsilon));
        return result;
    }

    bool choose_specular = (event < p.transmission + p.specular);
    float roughness = clamp01(material.roughness);
    float alpha = std::max(0.02f, roughness * roughness);

    for (int tries = 0; tries < 4; ++tries) {
        Vec3 wi;

        if (choose_specular) {
            Vec3 h = sample_ggx_half_vector(n, alpha);
            wi = normalize_or(reflect(-wo, h), n);
        } else {
            wi = sample_cosine_hemisphere(n);
        }

        if (dot(n, wi) <= 0.0f) {
            continue;
        }

        float mixed_pdf = pdf(wo, wi, hit, material);
        if (mixed_pdf <= kEpsilon) {
            continue;
        }

        Color f = eval(wo, wi, hit, material);
        if (f.near_zero()) {
            continue;
        }

        float cos_term = std::max(0.0f, dot(n, wi));
        result.wi = wi;
        result.pdf = mixed_pdf;
        result.valid = true;
        result.is_delta = false;
        result.weight = f * (cos_term / mixed_pdf);
        return result;
    }

    return result;
}

Color PbrBsdf::eval(const Vec3& wo_in, const Vec3& wi_in, const HitRecord& hit, const Material& material) const {
    Vec3 n = normalize_or(hit.normal, Vec3(0.0f, 1.0f, 0.0f));
    Vec3 wo = normalize_or(wo_in, n);
    Vec3 wi = normalize_or(wi_in, n);

    float n_dot_o = dot(n, wo);
    float n_dot_i = dot(n, wi);
    if (n_dot_o <= 0.0f || n_dot_i <= 0.0f) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    float transmission = clamp01(material.transmission);
    float metallic = clamp01(material.metallic);
    float roughness = clamp01(material.roughness);
    float alpha = std::max(0.02f, roughness * roughness);

    Color base = clamp_color01(material.base_color);
    Color diffuse = (1.0f - transmission) * (1.0f - metallic) * base *
                    (1.0f / 3.14159265358979323846f);

    Vec3 h = wo + wi;
    if (h.near_zero()) {
        return diffuse;
    }
    h = unit_vector(h);

    float n_dot_h = std::max(0.0f, dot(n, h));
    float o_dot_h = std::max(0.0f, dot(wo, h));

    float d = ggx_distribution(n_dot_h, alpha);
    float g = smith_geometry(n_dot_o, n_dot_i, alpha);
    Color f0 = compute_f0(material);
    Color f = schlick_fresnel(o_dot_h, f0);

    float denominator = std::max(4.0f * n_dot_o * n_dot_i, kEpsilon);
    Color specular = (1.0f - transmission) * f * (d * g / denominator);

    return diffuse + specular;
}

float PbrBsdf::pdf(const Vec3& wo_in, const Vec3& wi_in, const HitRecord& hit, const Material& material) const {
    Vec3 n = normalize_or(hit.normal, Vec3(0.0f, 1.0f, 0.0f));
    Vec3 wo = normalize_or(wo_in, n);
    Vec3 wi = normalize_or(wi_in, n);

    float n_dot_o = dot(n, wo);
    float n_dot_i = dot(n, wi);
    if (n_dot_o <= 0.0f || n_dot_i <= 0.0f) {
        return 0.0f;
    }

    SamplingProbabilities p = compute_sampling_probabilities(material);

    float diffuse_pdf = n_dot_i / 3.14159265358979323846f;
    float roughness = clamp01(material.roughness);
    float alpha = std::max(0.02f, roughness * roughness);
    float specular_pdf = ggx_reflection_pdf(wo, wi, n, alpha);

    return p.diffuse * diffuse_pdf + p.specular * specular_pdf;
}
