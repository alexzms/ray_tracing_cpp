//
// Created by alexzms on 2023/11/19.
//

#ifndef RAY_TRACING_SPHERE_H
#define RAY_TRACING_SPHERE_H

#include <utility>

#include "vec3.h"
#include "hittable.h"
#include "interval.h"
#include "material.h"

namespace geometry_objects {
    class sphere : public hittable {
    public:
        sphere(): center(), radius(0.0) {}
        sphere(const sphere &another) = default;
        sphere(point3 center, double radius, const std::shared_ptr<material>& obj_material):
                                center(std::move(center)), radius(radius), obj_material(obj_material) {
            if (!this->obj_material) {
                std::cout << "Warning: Sphere Material is empty" << std::endl;
            }
        }

        bool hit(const ray& r, const interval& inter, hit_record &rec) const override {
            // a = dir . dir = ||dir||_2^2 = 1, h_b = dir . (origin-center), c = ||origin- center||_2^2 - radius^2
            // h_discriminant = h_b - a*c
            vec3 oc = r.origin() - center;
            auto a = r.direction().length_square();         // must be +
            auto half_b = dot(r.direction(), oc);  // if collides, must be -
            auto c = oc.length_square() - radius * radius;  // not known, but normally it should be +
            auto discriminant = half_b * half_b - a * c;
            if (discriminant < 0) return false;
            auto sqrt_d = std::sqrt(discriminant);
            auto root = (-half_b - sqrt_d) / a;            // first we use the smaller solution(closest hit)
            if (!inter.surrounds(root)) {
                root = (-half_b + sqrt_d) / a;                     // switch to the larger solution
                if (!inter.surrounds(root)) {
                    return false;                                  // if it still not work, return false
                }
            }                                                      // now root must be within the range of t_min and t_max
            rec.t = root;
            rec.p = r.at(root);                                 // update the hit record
            rec.set_face_normal(r, (r.at(root) - center) / radius);
            rec.surface_material = obj_material;

            return true;
        }

    private:
        point3 center;
        double radius;
        std::shared_ptr<material> obj_material;
    };
}

#endif //RAY_TRACING_SPHERE_H
