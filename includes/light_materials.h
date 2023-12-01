//
// Created by alexzms on 2023/11/29.
//

#ifndef RAY_TRACING_LIGHT_MATERIALS_H
#define RAY_TRACING_LIGHT_MATERIALS_H
#include "texture.h"


namespace material {
    class diffuse_light : public material_base {
    public:
        explicit diffuse_light(std::shared_ptr<texture::texture_base> tex): emit_texture(std::move(tex)) {}
        explicit diffuse_light(const color& c): emit_texture(std::make_shared<texture::solid_color>(c)) {}
        bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const override {
            return false;
        }
        [[nodiscard]] color emitted(double u, double v, const point3& p) const override {
            return emit_texture->value(u, v, p);
        }
    private:
        std::shared_ptr<texture::texture_base> emit_texture;
    };
}

#endif //RAY_TRACING_LIGHT_MATERIALS_H
