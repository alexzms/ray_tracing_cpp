#ifndef RAY_TRACING_INTERVAL_H
#define RAY_TRACING_INTERVAL_H

#include "common.h"
#include "utilities.h"

class interval {
public:
    double min, max;
    interval(): min(+utilities::infinity), max(-utilities::infinity) {}         // default interval is empty
    interval(double min_, double max_): min(min_), max(max_) {}                 // constructor below merges two interval
    interval(const interval& i1, const interval& i2): min(fmin(i1.min, i2.min)), max(fmax(i1.max, i2.max)) {}

    [[nodiscard]] inline bool contains(double val) const {
        return min <= val && val <= max;
    }
    [[nodiscard]] inline bool surrounds(double val) const {
        return min < val && val < max;
    }
    [[nodiscard]] inline double clamp(double val) const {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }
    [[nodiscard]] inline double size() const {
        return max - min;
    }
    [[nodiscard]] inline interval expand(double delta) const {
        auto padding = delta / 2;
        return {min - padding, max + padding};
    }
    static const interval empty;
    static const interval universe;
};

interval operator+(const interval& ival, double displacement) {
    return interval(ival.min + displacement, ival.max + displacement);
}

interval operator+(double displacement, const interval& ival) {
    return ival + displacement;
}

const interval interval::empty(+utilities::infinity, -utilities::infinity);
const interval interval::universe(-utilities::infinity, +utilities::infinity);

#endif //RAY_TRACING_INTERVAL_H
