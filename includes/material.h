//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_MATERIAL_H
#define RAY_TRACING_MATERIAL_H

#include "hittable.h"

namespace material {
    class material_base {
    public:
        virtual ~material_base() = default;

        virtual bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const = 0;
        [[nodiscard]] virtual color emitted(double u, double v, const point3& p) const {
            return {0, 0, 0};                                                 // default is no emission
        }
    };
}


#endif //RAY_TRACING_MATERIAL_H
