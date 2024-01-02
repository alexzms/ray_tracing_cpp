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

        virtual bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out, double& pdf) const = 0;
        [[nodiscard]] virtual color emitted(double u, double v, const point3& p) const {
            return {0, 0, 0};                                                 // default is no emission
        }
        [[nodiscard]] virtual double scattering_pdf(const ray& in, const hit_record& rec, const ray& scattered) const {
            return 0.0;                                                                   // default is not a pdf
        }
    };
}


#endif //RAY_TRACING_MATERIAL_H
