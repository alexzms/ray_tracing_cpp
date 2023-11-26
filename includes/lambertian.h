//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_LAMBERTIAN_H
#define RAY_TRACING_LAMBERTIAN_H

#include <utility>

#include "material.h"
#include "texture.h"
#include "vec3.h"

class lambertian : public material {
public:
    explicit lambertian(const color& albedo): tex(std::make_shared<solid_color>(albedo)) {}
    explicit lambertian(std::shared_ptr<texture> tex): tex(std::move(tex)) {}

    bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const override {
        auto scatter_direction = rec.normal + vec3::random_unit_vec_on_sphere();  // lambertian scatter
        if (scatter_direction.near_zero()) {
            scatter_direction = rec.normal;
        }
        auto scatter_origin = rec.p;
        out = ray{scatter_origin, scatter_direction, in.time()};
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

private:
    std::shared_ptr<texture> tex;
};

#endif //RAY_TRACING_LAMBERTIAN_H
