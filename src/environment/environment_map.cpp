#include "environment_map.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kInvPi = 1.0f / kPi;
constexpr float kInvTwoPi = 1.0f / (2.0f * kPi);

inline void strip_cr(std::string& line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

inline Color rgbe_to_color(uint8_t r, uint8_t g, uint8_t b, uint8_t e) {
    if (e == 0) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    float scale = std::ldexp(1.0f, static_cast<int>(e) - (128 + 8));
    return Color(static_cast<float>(r) * scale,
                 static_cast<float>(g) * scale,
                 static_cast<float>(b) * scale);
}

inline bool decode_rle_scanline(std::ifstream& input, int scanline_width, std::vector<uint8_t>& out_rgbe) {
    uint8_t scanline_header[4] = {0, 0, 0, 0};
    input.read(reinterpret_cast<char*>(scanline_header), 4);
    if (!input) {
        return false;
    }

    if (scanline_header[0] != 2 || scanline_header[1] != 2 || (scanline_header[2] & 0x80) != 0) {
        return false;
    }

    int encoded_width = (static_cast<int>(scanline_header[2]) << 8) | static_cast<int>(scanline_header[3]);
    if (encoded_width != scanline_width) {
        return false;
    }

    std::vector<uint8_t> channel_data(4 * static_cast<size_t>(scanline_width), 0);

    for (int channel = 0; channel < 4; ++channel) {
        int x = 0;
        while (x < scanline_width) {
            uint8_t count_byte = 0;
            input.read(reinterpret_cast<char*>(&count_byte), 1);
            if (!input) {
                return false;
            }

            if (count_byte > 128) {
                int run_length = static_cast<int>(count_byte) - 128;
                if (run_length <= 0 || x + run_length > scanline_width) {
                    return false;
                }

                uint8_t value = 0;
                input.read(reinterpret_cast<char*>(&value), 1);
                if (!input) {
                    return false;
                }

                for (int i = 0; i < run_length; ++i) {
                    channel_data[static_cast<size_t>(channel) * static_cast<size_t>(scanline_width) + static_cast<size_t>(x + i)] = value;
                }
                x += run_length;
            } else {
                int literal_count = static_cast<int>(count_byte);
                if (literal_count <= 0 || x + literal_count > scanline_width) {
                    return false;
                }

                for (int i = 0; i < literal_count; ++i) {
                    uint8_t value = 0;
                    input.read(reinterpret_cast<char*>(&value), 1);
                    if (!input) {
                        return false;
                    }
                    channel_data[static_cast<size_t>(channel) * static_cast<size_t>(scanline_width) + static_cast<size_t>(x + i)] = value;
                }
                x += literal_count;
            }
        }
    }

    out_rgbe.resize(static_cast<size_t>(scanline_width) * 4u);
    for (int x = 0; x < scanline_width; ++x) {
        out_rgbe[static_cast<size_t>(x) * 4u + 0u] = channel_data[static_cast<size_t>(x)];
        out_rgbe[static_cast<size_t>(x) * 4u + 1u] = channel_data[static_cast<size_t>(scanline_width) + static_cast<size_t>(x)];
        out_rgbe[static_cast<size_t>(x) * 4u + 2u] = channel_data[static_cast<size_t>(2 * scanline_width) + static_cast<size_t>(x)];
        out_rgbe[static_cast<size_t>(x) * 4u + 3u] = channel_data[static_cast<size_t>(3 * scanline_width) + static_cast<size_t>(x)];
    }

    return true;
}

} // namespace

bool EnvironmentMap::load_hdr(const std::string& file_path) {
    width = 0;
    height = 0;
    pixels.clear();

    std::ifstream input(file_path, std::ios::binary);
    if (!input) {
        return false;
    }

    std::string line;
    if (!std::getline(input, line)) {
        return false;
    }
    strip_cr(line);

    if (line.rfind("#?", 0) != 0) {
        return false;
    }

    bool has_rle_format = false;
    while (std::getline(input, line)) {
        strip_cr(line);
        if (line.empty()) {
            break;
        }

        if (line.find("FORMAT=32-bit_rle_rgbe") != std::string::npos) {
            has_rle_format = true;
        }
    }

    if (!has_rle_format) {
        return false;
    }

    std::string resolution_line;
    if (!std::getline(input, resolution_line)) {
        return false;
    }
    strip_cr(resolution_line);

    std::string first_axis;
    std::string second_axis;
    int first_size = 0;
    int second_size = 0;

    std::stringstream ss(resolution_line);
    ss >> first_axis >> first_size >> second_axis >> second_size;

    if (!ss || first_size <= 0 || second_size <= 0) {
        return false;
    }

    bool y_major = (first_axis.size() == 2 && first_axis[1] == 'Y' && second_axis.size() == 2 && second_axis[1] == 'X');
    if (!y_major) {
        return false;
    }

    int parsed_height = first_size;
    int parsed_width = second_size;
    if (parsed_width < 8 || parsed_width > 32767) {
        return false;
    }

    std::vector<Color> loaded_pixels(static_cast<size_t>(parsed_width) * static_cast<size_t>(parsed_height), Color(0.0f, 0.0f, 0.0f));

    bool top_to_bottom = (first_axis[0] == '-');
    bool left_to_right = (second_axis[0] == '+');

    std::vector<uint8_t> scanline_rgbe;
    for (int scanline_index = 0; scanline_index < parsed_height; ++scanline_index) {
        if (!decode_rle_scanline(input, parsed_width, scanline_rgbe)) {
            return false;
        }

        int dst_y = top_to_bottom ? scanline_index : (parsed_height - 1 - scanline_index);

        for (int x = 0; x < parsed_width; ++x) {
            int dst_x = left_to_right ? x : (parsed_width - 1 - x);
            size_t src_index = static_cast<size_t>(x) * 4u;
            loaded_pixels[static_cast<size_t>(dst_y) * static_cast<size_t>(parsed_width) + static_cast<size_t>(dst_x)] =
                rgbe_to_color(scanline_rgbe[src_index + 0u],
                              scanline_rgbe[src_index + 1u],
                              scanline_rgbe[src_index + 2u],
                              scanline_rgbe[src_index + 3u]);
        }
    }

    width = parsed_width;
    height = parsed_height;
    pixels = std::move(loaded_pixels);
    return true;
}

Color EnvironmentMap::fetch(int x, int y) const {
    return pixels[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)];
}

Color EnvironmentMap::sample(const Vec3& direction) const {
    if (!is_valid()) {
        return Color(0.0f, 0.0f, 0.0f);
    }

    Vec3 dir = direction.near_zero() ? Vec3(0.0f, 1.0f, 0.0f) : unit_vector(direction);

    float u = std::atan2(dir.z, dir.x) * kInvTwoPi + 0.5f;
    u = u - std::floor(u);

    float y = std::clamp(dir.y, -1.0f, 1.0f);
    float v = std::acos(y) * kInvPi;
    v = std::clamp(v, 0.0f, 1.0f);

    float fx = u * static_cast<float>(width - 1);
    float fy = v * static_cast<float>(height - 1);

    int x0 = static_cast<int>(std::floor(fx));
    int y0 = static_cast<int>(std::floor(fy));
    int x1 = (x0 + 1) % width;
    int y1 = std::min(y0 + 1, height - 1);

    float tx = fx - static_cast<float>(x0);
    float ty = fy - static_cast<float>(y0);

    Color c00 = fetch(x0, y0);
    Color c10 = fetch(x1, y0);
    Color c01 = fetch(x0, y1);
    Color c11 = fetch(x1, y1);

    Color c0 = c00 * (1.0f - tx) + c10 * tx;
    Color c1 = c01 * (1.0f - tx) + c11 * tx;
    return (c0 * (1.0f - ty) + c1 * ty) * intensity;
}
