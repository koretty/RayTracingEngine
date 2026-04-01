#pragma once

#include "../math/vec3.hpp"
#include "../scene/scene.hpp"
#include "../scene/camera.hpp"
#include <vector>
#include <cstdint>
#include <memory>

class IBSDF;

class Renderer {
public:
    Renderer(int width, int height, int samples_per_pixel=10, int max_depth=5);
    Renderer(int width, int height, int samples_per_pixel, int max_depth, std::unique_ptr<IBSDF> bsdf_impl);
    ~Renderer();

    void render(const Scene& scene, const Camera& camera);

    const uint32_t* get_pixels() const { return pixels.data(); }

private:
    int width, height;
    int samples_per_pixel;
    int max_depth;
    std::vector<uint32_t> pixels;
    std::unique_ptr<IBSDF> bsdf;

    Vec3 evaluate_shadow_transmittance(const Scene& scene, const Ray& shadow_ray) const;
    Vec3 trace_ray(const Ray& ray, const Scene& scene, int depth) const;
    uint32_t to_color32(const Vec3& color) const;
};