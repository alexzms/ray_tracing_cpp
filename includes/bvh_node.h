//
// Created by alexzms on 2023/11/24.
//

#ifndef RAY_TRACING_BVH_NODE_H
#define RAY_TRACING_BVH_NODE_H

#include "hittable_list.h"
#include "hittable.h"
#include "aabb.h"
#include "memory"
#include "functional"

class bvh_node: public hittable {
public:
    explicit bvh_node(const hittable_list& world): bvh_node(world.objects, 0, world.objects.size()) {}
    bvh_node(const std::vector<std::shared_ptr<hittable>> &list, size_t start, size_t end) {
        std::vector<std::shared_ptr<hittable>> modifi_list = list;                        // copy the list

        int random_axis = utilities::random_int(0, 2);             // randomly choose an axis
        std::function<bool(const std::shared_ptr<hittable>&, std::shared_ptr<hittable>&)> comparator;
        if (random_axis == 0) {
            comparator = box_compare_x;
        } else if (random_axis == 1) {
            comparator = box_compare_y;
        } else {
            comparator = box_compare_z;
        }
//        auto comparator = (random_axis == 0) ? box_compare_x
//                                      : (random_axis == 1) ? box_compare_y
//                                                    : box_compare_z;

        size_t object_span = end - start;
        if (object_span == 1) {
            left = right = modifi_list[start];                                       // only 1 object in the span
        } else if (object_span == 2) {                                               // situation of span = 2
            if (comparator(modifi_list[start], modifi_list[start + 1])) {         // the reason of we specifically
                left = modifi_list[start];                                           // pick the situation of 2 out, is
                right = modifi_list[start + 1];                                      // we don't want to create more
            } else {                                                                 // bvh_node, we only need to store
                left = modifi_list[start + 1];                                       // left and write as hittable object
                right = modifi_list[start];
            }
        } else {
            std::sort(modifi_list.begin() + start, modifi_list.begin() + end, comparator);
            auto mid = start + object_span / 2;                              // sort and split to two
            left = std::make_shared<bvh_node>(modifi_list, start, mid);    // needs to be resorted
            right = std::make_shared<bvh_node>(modifi_list, mid, end);    // because we randomly choose axis
        }
        bbox = aabb(left->bounding_box(), right->bounding_box());
    }

    bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
        if (!bbox.hit(r, inter)) return false;
        bool left_hit = left->hit(r, inter, rec);          // if left_hit, there's no need to calculate what's behind
        bool right_hit = right->hit(r, interval(inter.min, left_hit? rec.t : inter.max), rec);

        return left_hit || right_hit;
    }

    [[nodiscard]] aabb bounding_box() const {
        return bbox;
    }


private:
    std::shared_ptr<hittable> left;
    std::shared_ptr<hittable> right;
    aabb bbox;

    static bool box_compare_axis(                                            // compare if a.axis is smaller than b.axis
        const std::shared_ptr<hittable> &a, const std::shared_ptr<hittable> &b, int axis_index) {
            return a->bounding_box().axis(axis_index).min < b->bounding_box().axis(axis_index).min;
    }
    static bool box_compare_x(                                               // compare if a.x is smaller than b.x
            const std::shared_ptr<hittable> &a, const std::shared_ptr<hittable> &b) {
        return box_compare_axis(a, b, 0);
    }
    static bool box_compare_y(                                               // compare if a.y is smaller than b.y
            const std::shared_ptr<hittable> &a, const std::shared_ptr<hittable> &b) {
        return box_compare_axis(a, b, 1);
    }
    static bool box_compare_z(                                               // compare if a.z is smaller than b.z
            const std::shared_ptr<hittable> &a, const std::shared_ptr<hittable> &b) {
        return box_compare_axis(a, b, 2);
    }
};

#endif //RAY_TRACING_BVH_NODE_H
