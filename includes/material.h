//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_MATERIAL_H
#define RAY_TRACING_MATERIAL_H

#include "hittable.h"

class material {
public:
    virtual ~material() = default;

    virtual bool scatter(const ray& in, const hit_record& rec, color& attenuation, ray& out) const = 0;
};


#endif //RAY_TRACING_MATERIAL_H
