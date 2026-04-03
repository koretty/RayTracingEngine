#pragma once

#include "../math/vec3.hpp"
#include <string>
#include <vector>

class EnvironmentMap {
public:
    bool load_hdr(const std::string& file_path);

    bool is_valid() const { return width > 0 && height > 0 && !pixels.empty(); }

    void set_intensity(float value) { intensity = value < 0.0f ? 0.0f : value; }
    float get_intensity() const { return intensity; }

    Color sample(const Vec3& direction) const;

    int get_width() const { return width; }
    int get_height() const { return height; }

private:
    int width{0};
    int height{0};
    float intensity{1.0f};
    std::vector<Color> pixels;

    Color fetch(int x, int y) const;
};
