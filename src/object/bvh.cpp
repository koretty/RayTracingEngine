#include "bvh.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace {

struct Bin {
    AABB bounds;
    int count{0};
};

inline float axis_value(const Vec3& v, int axis) {
    return v[axis];
}

} // namespace

BVHNode::BVHNode(const std::vector<std::shared_ptr<Object>>& src_objects, size_t start, size_t end)
    : Object(-1) {
    if (start >= end || end > src_objects.size()) {
        is_leaf = true;
        return;
    }

    std::vector<std::shared_ptr<Object>> objects;
    objects.reserve(end - start);
    objects.insert(objects.end(), src_objects.begin() + static_cast<std::ptrdiff_t>(start), src_objects.begin() + static_cast<std::ptrdiff_t>(end));

    if (objects.empty()) {
        is_leaf = true;
        return;
    }

    build(objects, 0u, objects.size());
}

BVHNode::BVHNode(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end)
    : Object(-1) {
    build(objects, start, end);
}

void BVHNode::make_leaf(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end) {
    is_leaf = true;
    left.reset();
    right.reset();
    leaf_objects.clear();

    if (start >= end) {
        return;
    }

    leaf_objects.reserve(end - start);
    box = AABB();

    for (size_t i = start; i < end; ++i) {
        const std::shared_ptr<Object>& obj = objects[i];
        if (!obj) {
            continue;
        }
        leaf_objects.push_back(obj);
        box.expand(obj->bounding_box());
    }
}

void BVHNode::build(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end) {
    if (start >= end) {
        is_leaf = true;
        return;
    }

    box = AABB();
    AABB centroid_bounds;
    size_t primitive_count = 0;

    for (size_t i = start; i < end; ++i) {
        const std::shared_ptr<Object>& obj = objects[i];
        if (!obj) {
            continue;
        }
        AABB obj_box = obj->bounding_box();
        box.expand(obj_box);
        centroid_bounds.expand(obj_box.centroid());
        ++primitive_count;
    }

    if (primitive_count == 0) {
        is_leaf = true;
        return;
    }

    if (primitive_count <= kLeafSize) {
        make_leaf(objects, start, end);
        return;
    }

    const Vec3 centroid_extent = centroid_bounds.extent();
    const float parent_area = box.surface_area();
    if (parent_area <= 0.0f) {
        make_leaf(objects, start, end);
        return;
    }

    float best_cost = std::numeric_limits<float>::infinity();
    int best_axis = -1;
    int best_split_bin = -1;

    for (int axis = 0; axis < 3; ++axis) {
        const float extent = axis_value(centroid_extent, axis);
        if (extent <= 1e-6f) {
            continue;
        }

        std::array<Bin, kBinCount> bins;
        const float inv_extent = static_cast<float>(kBinCount) / extent;
        const float cmin = centroid_bounds.pmin[axis];

        for (size_t i = start; i < end; ++i) {
            const std::shared_ptr<Object>& obj = objects[i];
            if (!obj) {
                continue;
            }

            const AABB obj_box = obj->bounding_box();
            const float c = obj_box.centroid()[axis];
            int bin_index = static_cast<int>((c - cmin) * inv_extent);
            bin_index = std::clamp(bin_index, 0, kBinCount - 1);

            bins[bin_index].count += 1;
            bins[bin_index].bounds.expand(obj_box);
        }

        std::array<AABB, kBinCount - 1> left_bounds;
        std::array<AABB, kBinCount - 1> right_bounds;
        std::array<int, kBinCount - 1> left_counts{};
        std::array<int, kBinCount - 1> right_counts{};

        AABB running_left;
        int running_left_count = 0;
        for (int i = 0; i < kBinCount - 1; ++i) {
            if (bins[i].count > 0) {
                running_left.expand(bins[i].bounds);
                running_left_count += bins[i].count;
            }
            left_bounds[i] = running_left;
            left_counts[i] = running_left_count;
        }

        AABB running_right;
        int running_right_count = 0;
        for (int i = kBinCount - 1; i > 0; --i) {
            if (bins[i].count > 0) {
                running_right.expand(bins[i].bounds);
                running_right_count += bins[i].count;
            }
            right_bounds[i - 1] = running_right;
            right_counts[i - 1] = running_right_count;
        }

        for (int split = 0; split < kBinCount - 1; ++split) {
            const int l_count = left_counts[split];
            const int r_count = right_counts[split];
            if (l_count == 0 || r_count == 0) {
                continue;
            }

            const float l_area = left_bounds[split].surface_area();
            const float r_area = right_bounds[split].surface_area();

            const float cost = 1.0f +
                               (l_area * static_cast<float>(l_count) + r_area * static_cast<float>(r_count)) /
                                   std::max(parent_area, 1e-12f);

            if (cost < best_cost) {
                best_cost = cost;
                best_axis = axis;
                best_split_bin = split;
            }
        }
    }

    const float leaf_cost = static_cast<float>(primitive_count);
    if (best_axis < 0 || best_cost >= leaf_cost) {
        make_leaf(objects, start, end);
        return;
    }

    split_axis = best_axis;
    const float axis_extent = centroid_extent[best_axis];
    const float inv_extent = axis_extent > 1e-6f ? static_cast<float>(kBinCount) / axis_extent : 0.0f;
    const float axis_min = centroid_bounds.pmin[best_axis];

    auto mid_it = std::partition(objects.begin() + static_cast<std::ptrdiff_t>(start),
                                 objects.begin() + static_cast<std::ptrdiff_t>(end),
                                 [&](const std::shared_ptr<Object>& obj) {
                                     if (!obj) {
                                         return false;
                                     }
                                     const float c = obj->bounding_box().centroid()[best_axis];
                                     int bin_index = static_cast<int>((c - axis_min) * inv_extent);
                                     bin_index = std::clamp(bin_index, 0, kBinCount - 1);
                                     return bin_index <= best_split_bin;
                                 });

    size_t mid = static_cast<size_t>(std::distance(objects.begin(), mid_it));
    if (mid == start || mid == end) {
        mid = start + (end - start) / 2;
        std::nth_element(objects.begin() + static_cast<std::ptrdiff_t>(start),
                         objects.begin() + static_cast<std::ptrdiff_t>(mid),
                         objects.begin() + static_cast<std::ptrdiff_t>(end),
                         [&](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) {
                             if (!a) {
                                 return false;
                             }
                             if (!b) {
                                 return true;
                             }
                             return a->bounding_box().centroid()[best_axis] < b->bounding_box().centroid()[best_axis];
                         });
    }

    if (mid == start || mid == end) {
        make_leaf(objects, start, end);
        return;
    }

    is_leaf = false;
    leaf_objects.clear();

    left = std::shared_ptr<Object>(new BVHNode(objects, start, mid));
    right = std::shared_ptr<Object>(new BVHNode(objects, mid, end));

    if (left && right) {
        box = AABB::merge(left->bounding_box(), right->bounding_box());
    } else if (left) {
        box = left->bounding_box();
    } else if (right) {
        box = right->bounding_box();
    }
}

bool BVHNode::hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const {
    if (!box.hit(r, t_min, t_max)) {
        return false;
    }

    if (is_leaf) {
        bool hit_anything = false;
        float closest_so_far = t_max;
        HitRecord temp_rec;

        for (const std::shared_ptr<Object>& obj : leaf_objects) {
            if (!obj) {
                continue;
            }

            if (obj->hit(r, t_min, closest_so_far, temp_rec)) {
                if (temp_rec.object_id < 0) {
                    temp_rec.object_id = obj->get_object_id();
                }
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    const Object* first = left.get();
    const Object* second = right.get();
    if (r.getDirection()[split_axis] < 0.0f) {
        std::swap(first, second);
    }

    bool hit_anything = false;
    float closest_so_far = t_max;
    HitRecord temp_rec;

    if (first && first->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
    }

    if (second && second->hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        rec = temp_rec;
    }

    return hit_anything;
}