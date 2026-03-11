#include "renderer.hpp"
#include "../math/math_utils.hpp"

Renderer::Renderer(int width, int height, int samples_per_pixel, int max_depth) : width(width), height(height), samples_per_pixel(samples_per_pixel), max_depth(max_depth){
    pixels.resize(width * height);
}

void Renderer::render(const Scene& scene, const Camera& camera) {
    int grid_size = static_cast<int>(std::sqrt(samples_per_pixel));
    int actual_samples = grid_size * grid_size;
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
            // Screen Y usually goes top-to-bottom, but rendering math assumes bottom-to-top.
            // Flip the Y coordinate when writing to the pixel buffer.
            int pixel_y = height - 1 - y;
            pixels[pixel_y * width + x] = to_color32(color * (1.0f / static_cast<float>(actual_samples)));
        }
    }
}

Vec3 Renderer::trace_ray(const Ray& ray, const Scene& scene, int depth) const {
    float t_min = 0.001f;
    float t_max = 1e10f;

    if (depth >= max_depth) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    HitRecord rec;
    if (scene.find_closest_hit(ray, t_min, t_max, rec)) {
        Vec3 normal = rec.normal;
        return 0.5f * Color(normal.x + 1.0f, normal.y + 1.0f, normal.z + 1.0f);
    }

    Vec3 unit_direction = unit_vector(ray.getDirection());
    float t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * Color(1.0f, 1.0f, 1.0f) + t * scene.get_background();
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

