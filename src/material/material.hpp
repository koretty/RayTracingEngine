#pragma once

#include "../math/vec3.hpp"
#include "../math/ray.hpp"
#include "../math/math_utils.hpp"
#include "../object/object.hpp"
#include <cmath>

struct Material {
    Color base_color{1.0f, 1.0f, 1.0f};
    float metallic{0.0f};
    float roughness{0.5f};
    float transmission{0.0f}; 
    float ior{1.5f};
    Color emission{0.0f, 0.0f, 0.0f};

    static inline float reflectance(float cosine, float ref_idx) {
        float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * std::pow((1.0f - cosine), 5.0f);
    }

    inline bool scatter(
        const Ray& r_in, 
        const HitRecord& rec, 
        Color& attenuation, 
        Ray& scattered
    ) const {
        if (random_float() < transmission) {
            Vec3 unit_dir = unit_vector(r_in.getDirection());
            Vec3 normal = rec.normal;
            float refraction_ratio = rec.front_face ? (1.0f / ior) : ior;

            float cos_theta = std::fmin(dot(-unit_dir, normal), 1.0f);
            float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
            bool cannot_refract = refraction_ratio * sin_theta > 1.0f;

            Vec3 direction;
            if (cannot_refract || reflectance(cos_theta, ior) > random_float()) {
                direction = reflect(unit_dir, normal);
            } else {
                direction = refract(unit_dir, normal, refraction_ratio);
            }

            direction = direction + roughness * random_unit_vector();
            if (direction.near_zero()) direction = normal;

            scattered = Ray(rec.point, direction);
            attenuation = base_color;
            return true;
        }

        Vec3 diffuse_dir = rec.normal + random_unit_vector();
        if (diffuse_dir.near_zero()) diffuse_dir = rec.normal;

        Vec3 reflected = reflect(unit_vector(r_in.getDirection()), rec.normal);
        Vec3 specular_dir = reflected + roughness * random_unit_vector();

        bool is_specular = (random_float() < metallic);
        Vec3 scatter_dir = is_specular ? specular_dir : diffuse_dir;

        scattered = Ray(rec.point, scatter_dir);
        attenuation = base_color;

        if (is_specular && dot(scattered.getDirection(), rec.normal) <= 0.0f) {
            return false;
        }
        return true;
    }


    static Material mirror() {
        return Material{Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f, 0.0f, 1.0f, Color(0,0,0)};
    }
    static Material glass(float ior = 1.5f) {
        return Material{Color(1.0f, 1.0f, 1.0f), 0.0f, 0.0f, 1.0f, ior, Color(0,0,0)};
    }
    static Material metal(const Color& color=Color(0.8f, 0.8f, 0.8f)) {
        return Material{color,1.0f, 0.1f, 0.0f, 1.0f, Color(0,0,0)};
    }
    static Material matte(const Color& color) {
        return Material{color, 0.0f, 1.0f, 0.0f, 1.0f, Color(0,0,0)};
    }
    static Material light(const Color& color, float intensity) {
        return Material{Color(0,0,0), 0.0f, 1.0f, 0.0f, 1.0f, color * intensity};
    }
};