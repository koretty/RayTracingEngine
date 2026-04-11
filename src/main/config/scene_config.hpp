#pragma once

#include "../../scene/scene.hpp"
#include "../../material/material.hpp"
#include "../../object/sphere.hpp"
#include "../../environment/environment_map.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

namespace config {
namespace environment {

inline bool enabled = false;
inline const char* hdr_path = "img/environment.hdr";
inline float intensity = 1.0f;

}

namespace scene {

namespace benchmark {

inline bool enabled = true;
inline int grid_half_extent = 5;       // Total count per axis = 2 * half + 1 (11x11 by default).
inline float sphere_radius = 0.22f;
inline float sphere_spacing = 0.55f;
inline float height_jitter = 0.06f;

} // namespace benchmark

inline Scene create_scene() {
    Scene sc;

    sc.set_background(Color(0.60f, 0.80f, 1.00f));

    Material ground;
    ground.albedo_texture = std::make_shared<CheckerTexture>(
        Color(0.32f, 0.50f, 0.27f),
        Color(0.24f, 0.38f, 0.20f),
        0.85f
    );
    ground.metallic = 0.0f;
    ground.roughness = 0.97f;

    Material brushed_metal;
    brushed_metal.albedo_texture = std::make_shared<SolidColor>(Color(0.9f, 0.85f, 0.8f));
    brushed_metal.metallic = 1.0f;
    brushed_metal.roughness = 0.2f;

    Material glass;
    glass.albedo_texture = std::make_shared<SolidColor>(Color(0.98f, 0.99f, 1.0f));
    glass.metallic = 0.0f;
    glass.roughness = 0.02f;
    glass.transmission = 1.0f;
    glass.ior = 1.5f;
    glass.absorption_coefficient = Color(5.0f, 5.0f, 10.0f);

    Material red_matte;
    auto matte_image = std::make_shared<ImageTexture>();
    if (matte_image->load("img/red_matte.ppm")) {
        red_matte.albedo_texture = matte_image;
    } else {
        red_matte.albedo_texture = std::make_shared<SolidColor>(Color(0.9f, 0.2f, 0.2f));
    }
    red_matte.metallic = 0.0f;
    red_matte.roughness = 0.8f;

    sc.add_material(ground);
    sc.add_material(brushed_metal);
    sc.add_material(glass);
    sc.add_material(red_matte);

    sc.add_object(std::make_unique<Sphere>(Point3(0.0f, -1000.0f, 0.0f), 1000.0f, 0));

    if (benchmark::enabled) {
        auto hash01 = [](int x, int z) {
            uint32_t hx = static_cast<uint32_t>(x) * 73856093u;
            uint32_t hz = static_cast<uint32_t>(z) * 19349663u;
            uint32_t h = hx ^ hz;
            h ^= (h >> 13);
            h *= 1274126177u;
            h ^= (h >> 16);
            return static_cast<float>(h & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
        };

        const int min_index = -benchmark::grid_half_extent;
        const int max_index = benchmark::grid_half_extent;

        int benchmark_count = 0;
        for (int iz = min_index; iz <= max_index; ++iz) {
            for (int ix = min_index; ix <= max_index; ++ix) {
                float jitter = (hash01(ix, iz) - 0.5f) * benchmark::height_jitter;
                float y = benchmark::sphere_radius + jitter;
                float x = static_cast<float>(ix) * benchmark::sphere_spacing;
                float z = static_cast<float>(iz) * benchmark::sphere_spacing;

                int selector = ((ix + iz) % 3 + 3) % 3;
                int material_id = 1;
                if (selector == 1) {
                    material_id = 3;
                } else if (selector == 2) {
                    material_id = 2;
                }

                sc.add_object(std::make_unique<Sphere>(Point3(x, y, z), benchmark::sphere_radius, material_id));
                ++benchmark_count;
            }
        }

        std::cout << "[Scene] Benchmark spheres: " << benchmark_count
                  << " (grid=" << (2 * benchmark::grid_half_extent + 1) << "x"
                  << (2 * benchmark::grid_half_extent + 1) << ")\n";
    } else {
        float spacing = 2.2f;
        sc.add_object(std::make_unique<Sphere>(Point3(-spacing, 0.5f, 1.0f), 0.5f, 1));
        sc.add_object(std::make_unique<Sphere>(Point3(0.0f, 0.5f, 0.0f), 0.5f, 2));
        sc.add_object(std::make_unique<Sphere>(Point3(spacing, 0.5f, -1.0f), 0.5f, 3));
    }

    sc.set_sun_direction(Vec3(-0.6f, -1.0f, -0.4f));
    sc.set_sun_intensity(0.5f);
    sc.set_sun_color(Color(1.00f, 0.97f, 0.88f));

    if (environment::enabled) {
        EnvironmentMap env;
        if (env.load_hdr(environment::hdr_path)) {
            env.set_intensity(environment::intensity);
            sc.set_environment_map(std::move(env));
        } else {
            std::cerr << "[EnvironmentMap] Failed to load HDRI: " << environment::hdr_path << "\n";
        }
    }

    return sc;
}

} 
} 
