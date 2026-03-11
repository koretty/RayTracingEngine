#pragma once
#include <vector>
#include <memory>
#include "../math/ray.hpp"      
#include "../object/object.hpp"
#include "../material/material.hpp"

class Scene {
public:
    Scene() = default;

    void add_object(std::unique_ptr<Object> object) {
        objects.push_back(std::move(object));
    }
    void set_background(const Color& bg) { background = bg; }
    void set_sun_direction(const Vec3& dir) { sun_direction = dir; }

    bool find_closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;

    const Color& get_background() const { return background; }
    const Vec3& get_sun_direction() const { return sun_direction; }
    Object* get_object(size_t index) { return objects[index].get(); }
    size_t get_object_count() const { return objects.size(); }

    void add_material(const Material& mat) {
        materials.push_back(mat);
    }
    const Material& get_material(int index) const {
        return materials[index];
    }
    size_t get_material_count() const { return materials.size(); }

private:
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<Material> materials;
    Color background = {0.2f, 0.7f, 0.8f};
    Vec3 sun_direction = {-1.0f, -1.0f, -1.0f};
};