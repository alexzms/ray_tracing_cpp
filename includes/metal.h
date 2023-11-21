//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_METAL_H
#define RAY_TRACING_METAL_H

#include <utility>

#include "material.h"

class metal: public material {
public:
    metal(color albedo):albedo(std::move(albedo)) {}

    bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const override {
        auto scatter_direction = reflect(in.direction(), rec.normal);
        auto scatter_origin = rec.p;
        out = ray{scatter_origin, scatter_direction};
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};


#endif //RAY_TRACING_METAL_H
