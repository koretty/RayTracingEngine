#include "scene.hpp"

bool Scene::find_closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    HitRecord temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;

    for (size_t object_index = 0; object_index < objects.size(); ++object_index) {
        if (objects[object_index]->hit(ray, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            temp_rec.object_id = static_cast<int>(object_index);
            rec = temp_rec;
        }
    }

    return hit_anything;
}
