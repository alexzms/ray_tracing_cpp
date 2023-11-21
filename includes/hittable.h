//
// Created by alexzms on 2023/11/18.
//

#ifndef RAY_TRACING_HITTABLE_H
#define RAY_TRACING_HITTABLE_H

#include "ray.h"
#include "utilities.h"
#include "interval.h"

class material;

class hit_record {
public:
    point3 p;
    vec3 normal;
    double t{};
    std::shared_ptr<material> surface_material;
    bool front_face{};

    hit_record() = default;
    hit_record(const hit_record& another) = default;
    hit_record(const hit_record&& another) noexcept {
        if (this == &another) return;
        p = another.p;
        normal = another.normal;
        t = another.t;
        front_face = another.front_face;
        surface_material = another.surface_material;
    }
    hit_record& operator = (const hit_record& another) {
        if (this == &another) return *this;
        p = another.p;
        normal = another.normal;
        t = another.t;
        front_face = another.front_face;
        surface_material = another.surface_material;
        return *this;
    }

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        // outward_normal should have length 1
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual ~hittable() = default;
    virtual bool hit(const ray& r, const interval &inter, hit_record &rec) const = 0;
};

#endif //RAY_TRACING_HITTABLE_H
