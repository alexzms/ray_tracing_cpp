//
// Created by alexzms on 2023/11/19.
//

#ifndef RAY_TRACING_HITTABLE_LIST_H
#define RAY_TRACING_HITTABLE_LIST_H

#include <utility>

#include "hittable.h"
#include "memory"
#include "vec3.h"
#include "vector"
#include "interval.h"
#include "aabb.h"

class hittable_list: public hittable {
public:
    std::vector<std::shared_ptr<hittable>> objects;

    hittable_list() = default;
    explicit hittable_list(std::shared_ptr<hittable> object) { add(std::move(object)); }

    void add(std::shared_ptr<hittable> object) {
        bbox = aabb(bbox, object->bounding_box());
        objects.emplace_back(std::move(object));
    }

    bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
        hit_record temp_rec;
        interval temp_interval(inter);
        bool hit_any = false;
//        auto closest_to_far = inter.max;

        for (const auto& object: objects) {
            if (object->hit(r, temp_interval, temp_rec)) {       // if the object can be hit within the
                hit_any = true;                                          // range of t_min and closest_to_far
                temp_interval.max = temp_rec.t;                          // closest_to_far is temp_interval.max
                rec = temp_rec;
            }
        }

        return hit_any;
    }

    [[nodiscard]] aabb bounding_box() const override {
        return bbox;
    }

private:
    aabb bbox;
};

#endif //RAY_TRACING_HITTABLE_LIST_H
