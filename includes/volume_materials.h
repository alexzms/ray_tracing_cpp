//
// Created by alexzms on 2023/11/30.
//

#ifndef RAY_TRACING_VOLUME_MATERIALS_H
#define RAY_TRACING_VOLUME_MATERIALS_H

#include "vec3.h"
#include "utilities.h"
#include "material.h"

namespace material::volume {
    class isotropic : public material_base {
    public:
        explicit isotropic(std::shared_ptr<texture::texture_base> texture) : texture(std::move(texture)) {}
        explicit isotropic(const color &c) : texture(std::make_shared<texture::solid_color>(c)) {}
        bool scatter(const ray &in, const hit_record &rec, color &attenuation, ray &out, double& pdf) const override {
            out = ray {rec.p, vec3::random_unit_vec_on_sphere(), in.time()};
            attenuation = texture->value(rec.u, rec.v, rec.p);
            pdf = 1 / (4 * utilities::pi);
            return true;
        }
        [[nodiscard]] double scattering_pdf(const ray& in, const hit_record& rec, const ray& scattered) const override{
            return 1 / (4 * utilities::pi);
        }

    private:
        std::shared_ptr<texture::texture_base> texture;
    };
}


#endif //RAY_TRACING_VOLUME_MATERIALS_H
