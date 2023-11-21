//
// Created by alexzms on 2023/11/18.
//

#ifndef RAY_TRACING_UTILITIES_H
#define RAY_TRACING_UTILITIES_H

#include "iostream"
#include "cmath"
#include "memory"
#include "limits"
#include "ray.h"
#include "vec3.h"
#include "random"

class interval;

namespace utilities {
    // Constants
    constexpr static double infinity = std::numeric_limits<double>::infinity();    // INFINITY is used, we use infinity
    constexpr static double pi = 3.1415926535897932385;                            // instead of INFINITY
    constexpr static double epsilon = 1e-8;                                        // epsilon for floating point comparison

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(0.0, 1.0);

    inline constexpr double degree_to_radian(double degree) {
        return degree * pi / 180.0;
    }
    inline double random_double() {
        return dis(gen);
    }
    inline double random_double(double min, double max) {
        return min + (max - min) * random_double();
    }
}

#endif //RAY_TRACING_UTILITIES_H
