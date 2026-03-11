#pragma once

#include "../math/vec3.hpp"
#include "../math/ray.hpp"
#include "../math/math_utils.hpp"
#include "../object/object.hpp"

struct Material {
    Color base_color{1.0f, 1.0f, 1.0f};
    float metallic{0.0f};
    float roughness{0.5f};
    float transmission{0.0f}; 
    float ior{1.5f};
    Color emission{0.0f, 0.0f, 0.0f};

    inline bool scatter(
        const Ray& r_in, 
        const HitRecord& rec, 
        Color& attenuation, 
        Ray& scattered
    ) const {
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
    static Material gold() {
        return Material{Color(1.0f, 0.84f, 0.0f), 1.0f, 0.1f, 0.0f, 1.0f, Color(0,0,0)};
    }
    static Material matte(const Color& color) {
        return Material{color, 0.0f, 1.0f, 0.0f, 1.0f, Color(0,0,0)};
    }
    static Material light(const Color& color, float intensity) {
        return Material{Color(0,0,0), 0.0f, 1.0f, 0.0f, 1.0f, color * intensity};
    }
};