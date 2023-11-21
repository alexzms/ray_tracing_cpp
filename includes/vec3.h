#ifndef VEC3_H
#define VEC3_H

#include "iostream"
#include "cmath"
#include "vector"
#include "utilities.h"

class vec3 {
public:
    double e[3];

    vec3(): e{0, 0, 0} {}
    vec3(double e1, double e2, double e3): e{e1, e2, e3} {}
    explicit vec3(double val): e{val, val, val} {}  // support for broadcasting

    // this is a copy constructor
    vec3(const vec3 &rhs): e{rhs.e[0], rhs.e[1], rhs.e[2]} {}
    // this is a move constructor
    vec3(const vec3 &&rhs) noexcept : e{rhs.e[0], rhs.e[1], rhs.e[2]} {}
    // assignment operator
    vec3& operator=(const vec3 &rhs) {
        if (this != &rhs) {
            e[0] = rhs.e[0];
            e[1] = rhs.e[1];
            e[2] = rhs.e[2];
        }
        return *this;
    }

    [[nodiscard]] double x() const { return e[0]; }
    [[nodiscard]] double y() const { return e[1]; }
    [[nodiscard]] double z() const { return e[2]; }

    // negative
    vec3 operator-() const { return {-e[0], -e[1], -e[2]}; }
    // subscript(const version, always returns copy)
    double operator[](int i) const { return e[i]; }
    // subscript(none-const version, returns reference, a reference can be modified, so shouldn't be a const)
    double& operator[](int i) { return e[i]; }

    vec3& operator += (const vec3 &rhs) {
        e[0] += rhs[0];
        e[1] += rhs[1];
        e[2] += rhs[2];
        return *this;
    }

    vec3& operator -= (const vec3 &rhs) {
        e[0] -= rhs[0];
        e[1] -= rhs[1];
        e[2] -= rhs[2];
        return *this;
    }

    // To be mentioned, we must use const reference here, c++ will not allow lhs(non-const) reference
    //   to be bound to a temporary object
    vec3& operator *= (const vec3 &rhs) {
        e[0] *= rhs[0];
        e[1] *= rhs[1];
        e[2] *= rhs[2];
        return *this;
    }

    vec3& operator *=(const double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator /= (const double t) {
        return (*this *= 1/t);
    }

    [[nodiscard]] double length_square() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    [[nodiscard]] double length() const {
        return std::sqrt(this->length_square());
    }

    void normalize() {
        *this /= this->length();
    }

    [[nodiscard]] bool near_zero() const {
        auto s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    static inline vec3 random_vec() {
        return {utilities::random_double(), utilities::random_double(), utilities::random_double()};
    }

    static inline vec3 random_vec(double min, double max) {
        return {utilities::random_double(min, max), utilities::random_double(min, max),
                                                        utilities::random_double(min, max)};
    }

    static inline vec3 random_unit_vec_on_sphere() {
        double a = utilities::random_double(0, 2 * utilities::pi);
        double z = utilities::random_double(-1, 1);
        double r = std::sqrt(1 - z * z);
        return {r * std::cos(a), r * std::sin(a), z};
    }

    static inline vec3 random_unit_vec_on_hemisphere(const vec3 &normal) {
        vec3 random_vec = random_unit_vec_on_sphere();
        if (dot(random_vec, normal) > 0.0) {
            return random_vec;
        } else {
            return -random_vec;
        }
    }

    friend std::ostream& operator <<(std::ostream &out, const vec3 &val);
    friend vec3 operator + (const vec3 &lhs, const vec3 &rhs);
    friend vec3 operator - (const vec3 &lhs, const vec3 &rhs);
    friend vec3 operator * (const vec3 &lhs, const vec3 &rhs);
    friend vec3 operator / (const vec3 &lhs, double t);
    friend double dot(const vec3 &lhs, const vec3 &rhs);
    friend vec3 cross(const vec3 &lhs, const vec3 &rhs);
    friend vec3 normalize(const vec3 &val);
    friend vec3 reflect(const vec3 &val, const vec3 &normal);
};

using point3 = vec3;

// inline instead of constexpr because constexpr requires compilers to calculate the return value when compiling
// but compiler cannot calculate stream
inline std::ostream& operator <<(std::ostream &out, const vec3 &val) {
    return out << val[0] << ' ' << val[1] << ' ' << val[2];
}

inline vec3 operator + (const vec3 &lhs, const vec3 &rhs) {
    return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

inline vec3 operator - (const vec3 &lhs, const vec3 &rhs) {
    return {lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]};
}

inline vec3 operator * (const vec3 &lhs, const vec3 &rhs) {
    return {lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2]};
}

inline vec3 operator * (const double &t, const vec3 &rhs) {
    return {t * rhs[0], t * rhs[1], t * rhs[2]};
}

inline vec3 operator * (const vec3 &lhs, const double &t) {
    return {t * lhs[0], t * lhs[1], t * lhs[2]};
}

inline vec3 operator / (const vec3 &lhs, double t) {
    return {lhs[0] / t, lhs[1] / t, lhs[2] / t};
}

inline double dot(const vec3 &lhs, const vec3 &rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

inline vec3 cross(const vec3 &lhs, const vec3 &rhs) {
    return {lhs[1] * rhs[2] - lhs[2] * rhs[1],
            lhs[2] * rhs[0] - lhs[0] * rhs[2],
            lhs[0] * rhs[1] - lhs[1] * rhs[0]};
}

inline vec3 normalize(const vec3 &val) {
    return val / val.length();
}

inline vec3 reflect(const vec3 &val, const vec3 &normal) {
    return val - 2 * dot(val, normal) * normal;
}

using normalize_func = vec3 (*) (const vec3&);
normalize_func unit_vector = normalize;


namespace unit_test {
    // unit test
    void vec3_test() {
        vec3 v1{1, 2, 3};
        std::cout << v1 << std::endl;
        vec3 v2{4, 5, 6};
//    v1 -= v2;
        vec3 v3 = 3 * v1;
        std::cout << v3 << std::endl;
        std::cout << reflect(normalize({1, -1, 0}), {0, 1, 0}) << std::endl;
    }
}

#endif //VEC3_H
