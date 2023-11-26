//
// Created by alexzms on 2023/11/18.
//

#ifndef RAY_H
#define RAY_H

#include <utility>

#include "vec3.h"

class ray {
public:
    ray(): ray_o(), ray_d() {}
    ray(point3 ray_o, vec3 ray_d): ray_o(std::move(ray_o)), ray_d(std::move(normalize(ray_d))), tm(0) {}
    ray(point3 ray_o, vec3 ray_d, double time):                                         // also init tm
                                ray_o(std::move(ray_o)), ray_d(std::move(normalize(ray_d))), tm(time) {}

    [[nodiscard]] point3 origin() const { return ray_o; }
    [[nodiscard]] vec3 direction() const { return ray_d; }
    [[nodiscard]] double time() const { return tm; }

    [[nodiscard]] point3 at(double t) const {
        return ray_o + t * ray_d;
    }

    friend inline std::ostream& operator << (std::ostream& out, const ray& r);

private:
    point3 ray_o;
    vec3 ray_d;
    double tm;
};

inline std::ostream& operator << (std::ostream& out, const ray& r) {
    out << "ray_o = " << r.ray_o.x() << ' ' << r.ray_o.y() << ' ' << r.ray_o.z() << "  "
        << "ray_d = " << r.ray_d.x() << ' ' << r.ray_d.y() << ' ' << r.ray_d.z() << '\n';
    return out;
}


namespace unit_test {
    void ray_test() {
        ray r(point3(1, 2, 3), vec3(4, 5, 6));
        std::cout << r.origin() << std::endl;
        std::cout << r.direction() << std::endl;
        std::cout << r.at(2) << std::endl;
    }
}

#endif //RAY_H
