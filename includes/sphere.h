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
#include "aabb.h"
#include "cmath"

namespace primitive {
    class sphere : public hittable {
    public:
        sphere(): center1(), radius(0.0), moving_obj(false), bbox() {}
        sphere(const sphere &another) = default;
        sphere(point3 center, double radius, const std::shared_ptr<material::material_base>& obj_material):    // stationary sphere
                center1(std::move(center)), radius(radius), obj_material(obj_material), moving_obj(false) {
            auto half_edge = vec3{radius, radius, radius};
            bbox = aabb{center1 - half_edge, center1 + half_edge};
        }

        sphere(const point3 &center, const point3 &center2, double radius, const std::shared_ptr<material::material_base>& obj_material):
                center1(center), center_moving_direction(center2 - center), radius(radius),
                obj_material(obj_material), moving_obj(true) {
            // TODO: add bvh split here
        }

        bool hit(const ray& r, const interval& inter, hit_record &rec) const override {
            // a = dir . dir = ||dir||_2^2 = 1, h_b = dir . (origin-center1), c = ||origin- center1||_2^2 - radius^2
            // h_discriminant = h_b - a*c
            point3 center = moving_obj ? get_center(r.time()) : center1;
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
            rec.surface_material = obj_material;
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            get_sphere_uv(outward_normal, rec.u, rec.v);

            return true;
        }

        [[nodiscard]] aabb bounding_box() const override { return bbox; }

    private:
        point3 center1;
        vec3 center_moving_direction;
        bool moving_obj;
        double radius;
        aabb bbox;
        std::shared_ptr<material::material_base> obj_material;

        point3 get_center(double time) const {
            if (!moving_obj) return center1;
            return center1 + time * center_moving_direction;      // center1 + time * (center2 - center1) interpolation
        }
        static void get_sphere_uv(const point3& p, double& u, double& v) {
            // p: a given point on the sphere of radius one, centered at the origin.
            // u: returned value [0,1] of angle around the Y axis from X=-1.
            // v: returned value [0,1] of angle from Y=-1 to Y=+1.
            //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
            //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
            //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

            auto theta = acos(-p.y());
            auto phi = atan2(-p.z(), p.x()) + utilities::pi;

            u = phi / (2 * utilities::pi);
            v = theta / utilities::pi;
        }
    };
}

#endif //RAY_TRACING_SPHERE_H
