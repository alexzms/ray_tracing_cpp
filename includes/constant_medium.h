//
// Created by alexzms on 2023/11/30.
//

#ifndef RAY_TRACING_CONSTANT_MEDIUM_H
#define RAY_TRACING_CONSTANT_MEDIUM_H

#include "hittable.h"
#include "utilities.h"
#include "texture.h"
#include "material.h"

class constant_medium: public hittable {
public:
    constant_medium(std::shared_ptr<hittable> boundary, double density,     // boundary, density and isotropic texture
        const std::shared_ptr<texture::texture_base>& texture): boundary(std::move(boundary)),
        negative_inv_density(-1/density), phase_function(std::make_shared<material::volume::isotropic>(texture)) {}
    constant_medium(std::shared_ptr<hittable> boundary, double density,     // boundary, density and isotropic color
        const color& color): boundary(std::move(boundary)),
        negative_inv_density(-1/density), phase_function(std::make_shared<material::volume::isotropic>(color)) {}

    [[nodiscard]] bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
        hit_record rec1, rec2;
        if (!boundary->hit(r, interval::universe, rec1))
            return false;                                                   // at least hit the boundary
        if (!boundary->hit(r, interval{rec1.t + 0.0001, utilities::infinity}, rec2))
            return false;                                                   // avoid very corner case

        if (rec1.t < inter.min) rec1.t = inter.min;
        if (rec2.t > inter.max) rec2.t = inter.max;                         // TODO: what the hell?

        if (rec1.t >= rec2.t) return false;                                 // TODO: is it possible?

        if (rec1.t < 0) rec1.t = 0;                                         // TODO: what the hell?

        auto distance_inside_boundary = rec2.t - rec1.t;            // TODO: what the hell?
        auto hit_distance = negative_inv_density * log(utilities::random_double());
        if (hit_distance > distance_inside_boundary) return false;          // TODO: what the hell?

        rec.t = rec1.t + hit_distance;
        rec.p = r.at(rec.t);
        rec.normal = vec3(1,0,0);                               // arbitrary
        rec.front_face = true;                                              // also arbitrary
        rec.surface_material = phase_function;

        return true;
    }

    [[nodiscard]] aabb bounding_box() const {
        return boundary->bounding_box();
    }



private:
    double negative_inv_density;
    std::shared_ptr<hittable> boundary;
    std::shared_ptr<material::material_base> phase_function;
};

#endif //RAY_TRACING_CONSTANT_MEDIUM_H
