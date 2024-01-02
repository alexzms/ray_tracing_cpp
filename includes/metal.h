//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_METAL_H
#define RAY_TRACING_METAL_H

#include <utility>

#include "material.h"

namespace material {
    class metal: public material::material_base {
    public:
        explicit metal(color albedo): albedo(std::move(albedo)), fuzz(0.0) {}
        metal(color albedo, double f): albedo(std::move(albedo)), fuzz(f) {}

        // TODO: pdf
        bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out, double& pdf) const override {
            auto scatter_direction = reflect(in.direction(), rec.normal) + fuzz * vec3::random_unit_vec_on_sphere();
            auto scatter_origin = rec.p;
            out = ray{scatter_origin, scatter_direction, in.time()};
            attenuation = albedo;
            return (dot(rec.normal, scatter_direction) > 0);
        }

    private:
        color albedo;
        double fuzz;
    };
}


#endif //RAY_TRACING_METAL_H
