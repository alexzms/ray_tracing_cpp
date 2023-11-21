#ifndef RAY_TRACING_INTERVAL_H
#define RAY_TRACING_INTERVAL_H

#include "common.h"
#include "utilities.h"

class interval {
public:
    double min, max;
    interval(): min(-utilities::infinity), max(utilities::infinity) {}
    interval(double min_, double max_): min(min_), max(max_) {}

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
    static const interval empty;
    static const interval universe;
};

const interval interval::empty(+utilities::infinity, -utilities::infinity);
const interval interval::universe(-utilities::infinity, +utilities::infinity);

#endif //RAY_TRACING_INTERVAL_H
