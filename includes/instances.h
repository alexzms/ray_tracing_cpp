//
// Created by alexzms on 2023/11/30.
//

#ifndef RAY_TRACING_INSTANCES_H
#define RAY_TRACING_INSTANCES_H

#include "vec3.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"

namespace instance {
    inline std::shared_ptr<hittable_list> box
                                (const point3& a, const point3& b, const std::shared_ptr<material::material_base>& mat) {
        // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

        auto sides = std::make_shared<hittable_list>();

        // Construct the two opposite vertices with the minimum and maximum coordinates.
        auto min = point3(fmin(a.x(), b.x()), fmin(a.y(), b.y()), fmin(a.z(), b.z()));
        auto max = point3(fmax(a.x(), b.x()), fmax(a.y(), b.y()), fmax(a.z(), b.z()));

        auto dx = vec3(max.x() - min.x(), 0, 0);
        auto dy = vec3(0, max.y() - min.y(), 0);
        auto dz = vec3(0, 0, max.z() - min.z());

        sides->add(make_shared<primitive::quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
        sides->add(make_shared<primitive::quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
        sides->add(make_shared<primitive::quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
        sides->add(make_shared<primitive::quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
        sides->add(make_shared<primitive::quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
        sides->add(make_shared<primitive::quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

        return sides;
    }


    class translate : public hittable {
    public:
        translate(std::shared_ptr<hittable> object, vec3 displacement):
                                        object(std::move(object)), offset(std::move(displacement)) {
            bbox = this->object->bounding_box() + offset;
        }

        bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
            ray offset_r(r.origin() - offset, r.direction(), r.time());     // move ray instead of obj

            if (!object->hit(offset_r, inter, rec))
                return false;
            rec.p += offset;                                                                  // if hit, move rec.p
            return true;
        }

        [[nodiscard]] aabb bounding_box() const override {
            return bbox;
        }

    private:
        std::shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
    };

    class rotate_y : public hittable {
    public:

        rotate_y(std::shared_ptr<hittable> object, double degree): object(std::move(object)) {
            auto radians = utilities::degree_to_radian(degree);
            sin_theta = sin(radians);
            cos_theta = cos(radians);
            bbox = this->object->bounding_box();                                             // copy construct
            point3 min(utilities::infinity);                                             // auto broadcast
            point3 max(-utilities::infinity);

            for (int i = 0; i < 2; i++) {                                                    // For every combination of
                for (int j = 0; j < 2; j++) {                                                // min, max of the original
                    for (int k = 0; k < 2; k++) {                                            // bbox, rotate and then
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;                    // store the value that will
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;                    // expand the bbox to the
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;                    // largest.

                        auto newx =  cos_theta*x + sin_theta*z;
                        auto newz = -sin_theta*x + cos_theta*z;

                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = fmin(min[c], tester[c]);
                            max[c] = fmax(max[c], tester[c]);
                        }
                    }
                }
            }
            bbox = aabb{min, max};
        }

        bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
            auto origin = r.origin();                                                // pipeline: w->o->w
            auto direction = r.direction();                                           // first world to object

            origin.x() = cos_theta * r.origin().x() - sin_theta * r.origin().z();           // x'=cos*x-sin*z
            origin.z() = sin_theta * r.origin().x() + cos_theta * r.origin().z();           // z'=sin*x+cos*z

            direction.x() = cos_theta * r.direction().x() - sin_theta * r.direction().z();  // same as ray origin
            direction.z() = sin_theta * r.direction().x() + cos_theta * r.direction().z();

            ray rotated_ray{origin, direction, r.time()};
            if (!object->hit(rotated_ray, inter, rec))
                return false;                                                               // test for object hit

            auto p = rec.p;                                                          // now object to world
            p.x() = cos_theta * p.x() + sin_theta * p.z();                                  // x'=cos*x+sin*z
            p.z() = -sin_theta * p.x() + cos_theta * p.z();                                 // z'=sin*x+cos*z
            auto normal = rec.normal;
            normal.x() = cos_theta * normal.x() + sin_theta * normal.z();                   // same for hit surface n
            normal.z() = -sin_theta * normal.x() + cos_theta * normal.z();

            rec.p = p;
            rec.normal = normal;
            return true;
        }

        [[nodiscard]] aabb bounding_box() const override {
            return bbox;
        }

    private:
        std::shared_ptr<hittable> object;
        double cos_theta;
        double sin_theta;
        aabb bbox;
    };
}

#endif //RAY_TRACING_INSTANCES_H
