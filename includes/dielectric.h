//
// Created by alexzms on 2023/11/21.
//

#ifndef RAY_TRACING_DIELECTRIC_H
#define RAY_TRACING_DIELECTRIC_H

#include "vec3.h"
#include "hittable.h"
#include "material.h"
#include "utilities.h"

class dielectric : public material {
public:
    explicit dielectric(double refract_coeff): refract_coeff(refract_coeff) {}

    bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const override {
        attenuation = color{1.0, 1.0, 1.0};                                 // full pass glass
        double refract_ratio = rec.front_face ? (1.0 / refract_coeff) : refract_coeff; // air to glass or vice versa
        double cos_theta = fmin(dot(-in.direction(), rec.normal), 1.0);
        double sin_theta = std::sqrt(1 - cos_theta * cos_theta);
        vec3 scatter_direction;
        point3 scatter_origin = rec.p;
        bool cannot_refract = refract_ratio * sin_theta > 1.0;                          // that will make sin_theta' > 1
        if (cannot_refract || reflectance(cos_theta, refract_coeff) > utilities::random_double()) {
            scatter_direction = reflect(in.direction(), rec.normal);               // we will reflect
        } else {                                                                       // otherwise refract
            scatter_direction = refract(in.direction(), rec.normal, refract_ratio);
        }

        out = ray(scatter_origin, scatter_direction, in.time());
        return true;
    }

private:
    double refract_coeff;
    static inline double reflectance(double cosine, double ref_idx) {
        auto r0 = (1-ref_idx) / (1+ref_idx);                                    // Schlick's approximation
        r0 = r0*r0;                                                                     // I copied the whole thing
        return r0 + (1-r0)*pow((1 - cosine),5);                                   // No idea how it works
    }
};

#endif //RAY_TRACING_DIELECTRIC_H
