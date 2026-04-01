#include "renderer.hpp"
#include "../bsdf/bsdf.hpp"
#include "../bsdf/pbr_bsdf.hpp"
#include "../material/optics.hpp"
#include "../math/math_utils.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <omp.h>

#ifndef RAYTRACER_ENABLE_BEER_LAMBERT
#define RAYTRACER_ENABLE_BEER_LAMBERT 1
#endif

namespace {

struct MediumEntry {
    int material_id{-1};
    float distance{0.0f};
};

} // namespace

Renderer::Renderer(int width, int height, int samples_per_pixel, int max_depth)
    : Renderer(width, height, samples_per_pixel, max_depth, std::make_unique<PbrBsdf>()) {
}

Renderer::Renderer(int width, int height, int samples_per_pixel, int max_depth, std::unique_ptr<IBSDF> bsdf_impl)
    : width(width),
      height(height),
      samples_per_pixel(samples_per_pixel),
      max_depth(max_depth),
      bsdf(std::move(bsdf_impl)) {
    if (!bsdf) {
        bsdf = std::make_unique<PbrBsdf>();
    }
    pixels.resize(width * height);
}

Renderer::~Renderer() = default;

void Renderer::render(const Scene& scene, const Camera& camera) {
#ifdef _OPENMP
    std::cout << "[OpenMP] threads: " << omp_get_max_threads() << "\n";
#else
    std::cout << "[OpenMP] disabled (single-thread)\n";
#endif

    int grid_size = static_cast<int>(std::sqrt(samples_per_pixel));
    int actual_samples = grid_size * grid_size;
    

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color color(0.0f, 0.0f, 0.0f);
            for (int sy = 0; sy < grid_size; sy++) {
                for (int sx = 0; sx < grid_size; sx++) {
                    float offset_x = (sx + random_float()) / grid_size;
                    float offset_y = (sy + random_float()) / grid_size;
                    float u = (static_cast<float>(x) + offset_x) / (width - 1);
                    float v = (static_cast<float>(y) + offset_y) / (height - 1);
                    Ray r = camera.get_ray(u, v);
                    color = color + trace_ray(r, scene, 0); 
                }
            }
            int pixel_y = height - 1 - y;
            Color pixel_color = color * (1.0f / static_cast<float>(actual_samples));
            pixel_color.x = std::sqrt(pixel_color.x);
            pixel_color.y = std::sqrt(pixel_color.y);
            pixel_color.z = std::sqrt(pixel_color.z);
            pixels[pixel_y * width + x] = to_color32(pixel_color);
        }
    }
}

Vec3 Renderer::evaluate_shadow_transmittance(const Scene& scene, const Ray& shadow_ray) const {
    constexpr float kShadowTMin = 0.001f;
    constexpr float kShadowTMax = 1e10f;
    constexpr float kShadowStepEpsilon = 0.001f;
    constexpr int kMaxShadowHits = 64;

    Color transmittance(1.0f, 1.0f, 1.0f);
    Ray current_ray = shadow_ray;
    float traveled_distance = 0.0f;
    float ray_dir_length = current_ray.getDirection().length();
    std::unordered_map<int, MediumEntry> active_media;

    if (ray_dir_length < 1e-6f) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    for (int i = 0; i < kMaxShadowHits; ++i) {
        HitRecord shadow_rec;
        if (!scene.find_closest_hit(current_ray, kShadowTMin, kShadowTMax, shadow_rec)) {
            break;
        }

        if (shadow_rec.material_id < 0 || static_cast<size_t>(shadow_rec.material_id) >= scene.get_material_count()) {
            return Color(0.0f, 0.0f, 0.0f);
        }

        const Material& blocker = scene.get_material(shadow_rec.material_id);
        if (blocker.transmission <= 0.0f) {
            return Color(0.0f, 0.0f, 0.0f);
        }

        float hit_distance = traveled_distance + shadow_rec.t * ray_dir_length;

#if RAYTRACER_ENABLE_BEER_LAMBERT
        if (shadow_rec.front_face) {
            active_media[shadow_rec.object_id] = MediumEntry{shadow_rec.material_id, hit_distance};
        } else {
            float entry_distance = traveled_distance;
            auto media_it = active_media.find(shadow_rec.object_id);
            if (media_it != active_media.end()) {
                entry_distance = media_it->second.distance;
                active_media.erase(media_it);
            }

            float thickness = std::max(0.0f, hit_distance - entry_distance);
            Color volume_transmittance = material_optics::beer_lambert_transmittance(blocker, thickness);
            transmittance = transmittance * volume_transmittance * blocker.transmission;
        }
#else
        Color pass_through = blocker.base_color * blocker.transmission;
        transmittance = transmittance * pass_through;
#endif

        if (transmittance.x < 0.001f && transmittance.y < 0.001f && transmittance.z < 0.001f) {
            return Color(0.0f, 0.0f, 0.0f);
        }

        traveled_distance = hit_distance;
        current_ray = Ray(shadow_rec.point + current_ray.getDirection() * kShadowStepEpsilon, current_ray.getDirection());
    }

    return transmittance;
}

Vec3 Renderer::trace_ray(const Ray& ray, const Scene& scene, int depth) const {
    if (depth >= max_depth) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    HitRecord rec;
    if (scene.find_closest_hit(ray, 0.001f, 1e10f, rec)) {
        if (rec.material_id >= 0 && static_cast<size_t>(rec.material_id) < scene.get_material_count()) {
            const Material& mat = scene.get_material(rec.material_id);

            Color emitted = mat.emission;
            Color direct_sun(0.0f, 0.0f, 0.0f);
            Vec3 wo = -unit_vector(ray.getDirection());

            Vec3 sun_dir = scene.get_sun_direction();
            if (!sun_dir.near_zero()) {
                Vec3 wi = -unit_vector(sun_dir);
                float n_dot_l = dot(rec.normal, wi);
                if (n_dot_l > 0.0f) {
                    Ray shadow_ray(rec.point + rec.normal * 0.001f, wi);
                    Color shadow_visibility = evaluate_shadow_transmittance(scene, shadow_ray);
                    Color f = bsdf->eval(wo, wi, rec, mat);
                    direct_sun = f * scene.get_sun_color() * (scene.get_sun_intensity() * n_dot_l) * shadow_visibility;
                }
            }

            BsdfSample sampled = bsdf->sample(wo, rec, mat);
            if (sampled.valid) {
                Ray next_ray(rec.point + sampled.wi * 0.001f, sampled.wi);
                return emitted + direct_sun + sampled.weight * trace_ray(next_ray, scene, depth + 1);
            }
            return emitted + direct_sun;
        }

        Vec3 normal = rec.normal;
        return 0.5f * Color(normal.x + 1.0f, normal.y + 1.0f, normal.z + 1.0f);
    }

    return scene.get_background();
}

uint32_t Renderer::to_color32(const Vec3& color) const {
    float rf = color.x * 255.999f;
    float gf = color.y * 255.999f;
    float bf = color.z * 255.999f;
    int r = rf < 0.0f ? 0 : (rf > 255.0f ? 255 : static_cast<int>(rf));
    int g = gf < 0.0f ? 0 : (gf > 255.0f ? 255 : static_cast<int>(gf));
    int b = bf < 0.0f ? 0 : (bf > 255.0f ? 255 : static_cast<int>(bf));

    return (255u << 24) | (r << 16) | (g << 8) | b;
}

