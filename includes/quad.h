//
// Created by alexzms on 2023/11/29.
//

#ifndef RAY_TRACING_QUAD_H
#define RAY_TRACING_QUAD_H

#include "vec3.h"
#include "interval.h"
#include "aabb.h"
#include "hittable.h"
// I do think life's ultimate goal is to make a quad(written by copilot)
namespace primitive {
    class quad : public hittable {
    public:
        quad(point3 Q, vec3 u, vec3 v, std::shared_ptr<material::material_base> obj_material):
                    Q(std::move(Q)), u(std::move(u)), v(std::move(v)), obj_material(std::move(obj_material)) {

            bbox = aabb(this->Q, this->Q + this->u + this->v);                // bounding box
            auto n = cross(this->u, this->v);                          // pull out the n, because of w
            normal = normalize(n);                                               // normal = cross(u, v)
            D = dot(normal, this->Q);
            w = n / dot(n, n);                                               // w is a common vec3 in calc step
        }

        virtual void set_bounding_box() {                                            // TODO: why virtual?
            bbox = aabb(Q, Q + u + v);
        }

        [[nodiscard]] aabb bounding_box() const override {
            return bbox;
        }

        bool hit(const ray& r, const interval &inter, hit_record &rec) const override {
            auto denominator = dot(normal, r.direction());
            if (fabs(denominator) < utilities::epsilon)
                return false;
            auto t = (D - dot(normal, r.origin())) / denominator;
            if (!inter.contains(t))
                return false;

            auto P = r.at(t);

            vec3 p = P - Q;                                                             // p is Q->P vector
            auto alpha = dot(w, cross(p, v));                  // p's length on basis u(not v!)
            auto beta  = dot(w, cross(u, p));                  // p's length on basis v(not u!)

            bool is_inside = is_interior(alpha, beta, rec);                    // judge and update rec
            if (!is_inside) return false;

            rec.t = t;                                                                  // if hit, update rec
            rec.p = P;
            rec.surface_material = this->obj_material;
            rec.set_face_normal(r, normal);

            return true;
        }

        [[nodiscard]] virtual inline bool is_interior(double a, double b, hit_record& rec) const {
            // given the length on basis u and v, update the rec's material u,v index(not the same concept of u, v!)
            // return if the hit point is inside the primitive
            if ((a < 0) || (a > 1) || (b < 0) || (b > 1))                  // TODO: can this if optimized by statistics?
                return false;

            rec.u = a;
            rec.v = b;
            return true;
        }


    private:
        point3 Q;                                                                   // start-point(bottom-left)
        vec3 u, v;                                                                  // basis vector(span the quad)
        aabb bbox;                                                                  // bounding box
        std::shared_ptr<material::material_base> obj_material;                      // material
        vec3 normal;                                                                // normalize(cross(u, v)) = normal
        double D;                                                                 // Ax+By+Cz=D
        vec3 w;
    };
}

#endif //RAY_TRACING_QUAD_H
