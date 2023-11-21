//
// Created by alexzms on 2023/11/18.
//

#ifndef COLOR_H
#define COLOR_H
#include "vec3.h"
#include "iostream"
using color = vec3;

inline double linear_to_gamma(double val) {
    return std::sqrt(val);
}

void write_color(std::ostream &out, const color& c, unsigned int samples_per_pixel = 1, unsigned int depth = 255) {
    static const interval intensity(0, 0.999);
    auto r = c[0], g = c[1], b = c[2];
    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);
    // Write the translated [0,255] value of each color component.
    double coefficient = depth + 0.999;
    out << static_cast<int>(intensity.clamp(r) * coefficient) << ' '
        << static_cast<int>(intensity.clamp(g) * coefficient) << ' '
        << static_cast<int>(intensity.clamp(b) * coefficient) << '\n';
}

#endif //COLOR_H
