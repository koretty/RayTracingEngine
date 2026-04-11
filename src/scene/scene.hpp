#pragma once
#include <vector>
#include <memory>
#include <utility>
#include "../math/ray.hpp"      
#include "../object/object.hpp"
#include "../material/material.hpp"
#include "../environment/environment_map.hpp"

class Scene {
public:
    Scene() = default;

    void add_object(std::unique_ptr<Object> object) {
        if (!object) {
            return;
        }
        object->set_object_id(static_cast<int>(objects.size()));
        objects.push_back(std::shared_ptr<Object>(std::move(object)));
        bvh_dirty = true;
    }
    void add_material(const Material& mat) {
        materials.push_back(mat);
    }
    void set_background(const Color& bg) { background = bg; }
    void set_environment_map(EnvironmentMap environment) {
        has_environment = environment.is_valid();
        environment_map = std::move(environment);
    }
    void clear_environment_map() {
        environment_map = EnvironmentMap();
        has_environment = false;
    }
    bool has_environment_map() const { return has_environment; }
    Color sample_environment(const Vec3& direction) const {
        if (has_environment) {
            return environment_map.sample(direction);
        }
        return background;
    }
    void set_sun_direction(const Vec3& dir) {
        if (dir.near_zero()) {
            return;
        }
        sun_direction = unit_vector(dir);
    }
    void set_sun_intensity(float i) { if (i < 0.0f) i = 0.0f; sun_intensity = i; }
    float get_sun_intensity() const { return sun_intensity; }
    void set_sun_color(const Color& c) { sun_color = c; }
    const Color& get_sun_color() const { return sun_color; }

    bool find_closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;

    const Color& get_background() const { return background; }
    const Vec3& get_sun_direction() const { return sun_direction; }
    Object* get_object(size_t index) { return objects[index].get(); }
    size_t get_object_count() const { return objects.size(); }
    const Material& get_material(int index) const { return materials[index]; }
    size_t get_material_count() const { return materials.size(); }

private:
    void build_bvh() const;

    std::vector<std::shared_ptr<Object>> objects;
    std::vector<Material> materials;
    Color background = {0.2f, 0.7f, 0.8f};
    EnvironmentMap environment_map;
    bool has_environment{false};
    Vec3 sun_direction = unit_vector(Vec3(-1.0f, -1.0f, -1.0f));
    float sun_intensity = 1.8f;
    Color sun_color = Color(1.0f, 0.97f, 0.92f);
    mutable std::shared_ptr<Object> bvh_root;
    mutable bool bvh_dirty{true};
};