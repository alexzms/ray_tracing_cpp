//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_LAMBERTIAN_H
#define RAY_TRACING_LAMBERTIAN_H

#include <utility>

#include "material.h"
#include "texture.h"
#include "vec3.h"
#include "onb.h"

namespace material {
    class lambertian : public material::material_base {
    public:
        explicit lambertian(const color& albedo): tex(std::make_shared<texture::solid_color>(albedo)) {}
        explicit lambertian(std::shared_ptr<texture::texture_base> tex): tex(std::move(tex)) {}

        bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out, double& pdf) const override {
//                                                                            // already satisfying the scattering_pdf
//            auto scatter_direction = rec.normal + vec3::random_unit_vec_on_sphere();  // lambertian scatter

            onb uvw;
            uvw.build_from_normal(rec.normal);                                // lambertian distribution(cosine distri)
            auto scatter_direction = uvw.local_to_global(random_cosine_direction());

            if (scatter_direction.near_zero()) {
                scatter_direction = rec.normal;
            }
            auto scatter_origin = rec.p;
            out = ray{scatter_origin, scatter_direction, in.time()};
            attenuation = tex->value(rec.u, rec.v, rec.p);
            pdf = dot(uvw.w(), out.direction()) / utilities::pi;    // pdf=cos(theta)/pi
            return true;
        }

        [[nodiscard]] double scattering_pdf(const ray& in, const hit_record& rec, const ray& scattered) const override {
            onb uvw;
            uvw.build_from_normal(rec.normal);
            return dot(uvw.w(), scattered.direction()) / utilities::pi;
            auto normal = rec.normal;
            auto cos_theta = dot(normal, scattered.direction());
            return cos_theta > 0 ? cos_theta/utilities::pi :  0;                           // pdf=cos(theta)/pi
        }

    private:
        std::shared_ptr<texture::texture_base> tex;
    };
}

#endif //RAY_TRACING_LAMBERTIAN_H
