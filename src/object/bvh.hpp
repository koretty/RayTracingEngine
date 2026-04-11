#pragma once

#include "object.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class BVHNode final : public Object {
public:
    BVHNode(const std::vector<std::shared_ptr<Object>>& src_objects, size_t start, size_t end);

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec) const override;
    AABB bounding_box() const override { return box; }

private:
    static constexpr size_t kLeafSize = 4;
    static constexpr int kBinCount = 16;

    BVHNode(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end);

    void build(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end);
    void make_leaf(std::vector<std::shared_ptr<Object>>& objects, size_t start, size_t end);

    AABB box;
    std::shared_ptr<Object> left;
    std::shared_ptr<Object> right;
    std::vector<std::shared_ptr<Object>> leaf_objects;
    int split_axis{0};
    bool is_leaf{false};
};