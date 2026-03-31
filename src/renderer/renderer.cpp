#include "renderer.hpp"
#include "../math/math_utils.hpp"
#include <cmath>
#include <iostream>
#include <omp.h>

Renderer::Renderer(int width, int height, int samples_per_pixel, int max_depth) : width(width), height(height), samples_per_pixel(samples_per_pixel), max_depth(max_depth){
    pixels.resize(width * height);
}

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

Vec3 Renderer::trace_ray(const Ray& ray, const Scene& scene, int depth) const {
    if (depth >= max_depth) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    HitRecord rec;
    if (scene.find_closest_hit(ray, 0.001f, 1e10f, rec)) {
        if (rec.material_id >= 0 && static_cast<size_t>(rec.material_id) < scene.get_material_count()) {
            const Material& mat = scene.get_material(rec.material_id);
            Color attenuation;
            Ray scattered(Point3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f));
            
            Color emitted = mat.emission;
            Color direct_sun(0.0f, 0.0f, 0.0f);

            Vec3 sun_dir = scene.get_sun_direction();
            if (!sun_dir.near_zero()) {
                Vec3 light_dir = -sun_dir;
                float ndotl = dot(rec.normal, light_dir);
                if (ndotl > 0.0f) {
                    Ray shadow_ray(rec.point + rec.normal * 0.001f, light_dir);
                    HitRecord shadow_rec;
                    bool blocked = scene.find_closest_hit(shadow_ray, 0.001f, 1e10f, shadow_rec);
                    if (!blocked) {
                        Color sun_color = scene.get_sun_color();
                        float sun_intensity = scene.get_sun_intensity();
                        float diffuse_weight = (1.0f - mat.metallic) * (1.0f - mat.transmission);
                        direct_sun = mat.base_color * sun_color * (sun_intensity * ndotl * diffuse_weight);
                    }
                }
            }
            
            if (mat.scatter(ray, rec, attenuation, scattered)) {
                return emitted + direct_sun + attenuation * trace_ray(scattered, scene, depth + 1);
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

