//
// Created by alexzms on 2023/11/23.
//

#ifndef RAY_TRACING_AABB_H
#define RAY_TRACING_AABB_H

#include "vec3.h"
#include "interval.h"

class aabb {
public:
    interval x, y, z;
    aabb() = default;                                                          // default: intervals are all universe
    aabb(const interval& ix, const interval& iy, const interval& iz): x(ix), y(iy), z(iz) {}
    aabb(const point3& p1, const point3& p2) {
        x = interval(fmin(p1[0], p2[0]), fmax(p1[0], p2[0]));
        y = interval(fmin(p1[1], p2[1]), fmax(p1[1], p2[1]));
        z = interval(fmin(p1[2], p2[2]), fmax(p1[2], p2[2]));
    }
    aabb(const aabb& b1, const aabb& b2) {
        x = interval{b1.x, b2.x};                                       // interval constructor will automatically
        y = interval{b1.y, b2.y};                                       // merge the two intervals
        z = interval{b1.z, b2.z};
    }

    [[nodiscard]] const interval& axis(int dim) const {
        if (dim == 0) return x;
        if (dim == 1) return y;
        if (dim == 2) return z;
        return z;
    }

    // This version works better with compiler, although it's mathematically the same with hit_my_version
    [[nodiscard]] bool hit(const ray& r, interval ray_t) const {
        for (int a = 0; a < 3; a++) {
            auto invD = 1 / r.direction()[a];                             // store the 1/direction
            auto orig = r.origin()[a];                                    // also store this
            auto t0 = (axis(a).min - orig) * invD;                   // don't need to calculate those again
            auto t1 = (axis(a).max - orig) * invD;
            if (invD < 0)                                                         // the case of negative ray
                std::swap(t0, t1);                                          // code like this avoids min/max()
            if (t0 > ray_t.min) ray_t.min = t0;                                   // use 'if' instead of fmax, fmin
            if (t1 < ray_t.max) ray_t.max = t1;                                   // is more compiler friendly I guess
            if (ray_t.max <= ray_t.min)                                           // corner is considered no
                return false;
        }
        return true;
    }

    [[deprecated]][[nodiscard]] bool hit_my_version(const ray& r, interval ray_t) const {
        for (int axis_i = 0; axis_i != 3; ++axis_i) {
            auto t0 = fmin( (axis(axis_i).min - r.origin().x())/r.direction().x() ,// t0 = min( (x0 - Ax)/bx),
                            (axis(axis_i).max - r.origin().x())/r.direction().x());        //           (x1 - Ax)/bx)
            auto t1 = fmax( (axis(axis_i).min - r.origin().x())/r.direction().x() ,// t1 = min( (x0 - Ax)/bx),
                            (axis(axis_i).max - r.origin().x())/r.direction().x());        //           (x1 - Ax)/bx)
            ray_t.min = fmax(t0, ray_t.min);
            ray_t.max = fmin(t1, ray_t.max);
            if (ray_t.max <= ray_t.min) return false;                                           // corner is considered no
        }
        return true;
    }
};

#endif //RAY_TRACING_AABB_H
