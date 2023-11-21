/*
 * This program will output a PPM Image Format
 */

#include "iostream"
#include "fstream"
#include "./includes/common.h"


//double hit_sphere(const ray& r, const point3& sphere_center, const double& radius) {
//    // a = dir . dir = ||dir||_2^2 = 1, b = 2 * dir . (origin-center), c = ||origin- center||_2^2 - radius^2
//    vec3 oc = r.origin() - sphere_center;
//    auto a = 1.0;
//    auto half_b = dot(oc, r.direction());
//    auto c = oc.length_square() - radius * radius;
//    auto discriminant = half_b * half_b - a * c;
//    if (discriminant >= 0) {
//        return (-half_b - std::sqrt(discriminant)) / a;
//    }
//    return -1.0;
//}

//color ray_color(const ray &r, const hittable& world) {
//    hit_record rec;
//    if (world.hit(r, interval(0, utilities::infinity), rec)) {
//        return 0.5 * (rec.normal + color(1, 1, 1));
//    }
//    // focal length is 1.0, viewport_v is 2.0, so if direction is not normalized, y is [-1, 1], but since
//    // it's normalized, there's also a z=1.0, therefore the maximum of y will be at the point of right_most_upper corner
//    // that will result in 0.77 in our case, check out learning/axis.png
//    auto blend_factor = 0.5 * (r.direction().y() + 1.0);
//    color color1{1.0, 1.0, 1.0};
//    color color2{0.5, 0.7, 1.0};
//    return (1 - blend_factor) * color1 + blend_factor * color2;
//}


int main() {
    // create materials
    auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto material_left   = std::make_shared<metal>(color(0.8, 0.8, 0.8));
    auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2));

    // create world scene
    hittable_list world;
    world.add(make_shared<geometry_objects::sphere>
            (point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<geometry_objects::sphere>
            (point3( 0.0,    0.5, -1.0),   0.3, material_center));
    world.add(make_shared<geometry_objects::sphere>
            (point3(-0.6,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<geometry_objects::sphere>
            (point3( 0.6,    0.0, -1.0),   0.5, material_right));

    camera cam;
    cam.set_camera_parameter(16.0 / 9.0, 800);
    cam.sample_per_pixel = 500;
    cam.max_depth = 50;
    cam.set_output_file("output.ppm");

    cam.render(world);

    return 0;
}