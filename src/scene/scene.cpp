#include "scene.hpp"
#include "../object/bvh.hpp"

void Scene::build_bvh() const {
    if (objects.empty()) {
        bvh_root.reset();
        bvh_dirty = false;
        return;
    }

    bvh_root = std::make_shared<BVHNode>(objects, 0u, objects.size());
    bvh_dirty = false;
}

bool Scene::find_closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    if (objects.empty()) {
        return false;
    }

    if (bvh_dirty || !bvh_root) {
        build_bvh();
    }

    if (bvh_root && bvh_root->hit(ray, t_min, t_max, rec)) {
        return true;
    }

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
