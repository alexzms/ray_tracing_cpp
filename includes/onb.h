//
// Created by alexzms on 2023/12/7.
//

#ifndef RAY_TRACING_ONB_H
#define RAY_TRACING_ONB_H

#include "vec3.h"
#include "utilities.h"

class onb {
public:
    onb() = default;                                    // constructor constructed all zero axis, which is invalid

    vec3& operator[](int index) { return axis[index]; }
    vec3 operator[](int index) const { return axis[index]; }

    [[nodiscard]] vec3 u() const { return axis[0]; }
    [[nodiscard]] vec3 v() const { return axis[1]; }
    [[nodiscard]] vec3 w() const { return axis[2]; }

    [[nodiscard]] vec3& u() { return axis[0]; }
    [[nodiscard]] vec3& v() { return axis[1]; }
    [[nodiscard]] vec3& w() { return axis[2]; }

    vec3 local_to_global(double x, double y, double z) { return axis[0] * x + axis[1] * y + axis[2] * z; }
    vec3 local_to_global(const vec3& local_coords) {
        return axis[0] * local_coords.x() + axis[1] * local_coords.y() + axis[2] * local_coords.z();
    }

    void build_from_normal(const vec3& normal) {
        vec3 normalized_normal = normalize(normal);
        vec3 a = (fabs(normalized_normal.x()) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
        vec3 v = normalize(cross(a, normalized_normal));
        vec3 u = cross(normalized_normal, v);
        axis[0] = u;
        axis[1] = v;
        axis[2] = normalized_normal;
    }


private:
    vec3 axis[3];
};

#endif //RAY_TRACING_ONB_H
