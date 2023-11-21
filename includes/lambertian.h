//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_LAMBERTIAN_H
#define RAY_TRACING_LAMBERTIAN_H

#include <utility>

#include "material.h"
#include "vec3.h"

class lambertian : public material {
public:
    explicit lambertian(color albedo): albedo(std::move(albedo)) {}

    bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const override {
        auto scatter_direction = rec.normal + vec3::random_unit_vec_on_sphere();  // lambertian scatter
        if (scatter_direction.near_zero()) {
            scatter_direction = rec.normal;
        }
        auto scatter_origin = rec.p;
        out = ray{scatter_origin, scatter_direction};
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

#endif //RAY_TRACING_LAMBERTIAN_H
