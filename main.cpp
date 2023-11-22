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

int sample_scene() {
    // create materials
    auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto material_left   = std::make_shared<lambertian>(color(0.9, 0.9, 0.9));
    auto material_right  = std::make_shared<metal>(color(0.95, 0.95, 0.95), 0.0);
    auto material_behind = std::make_shared<lambertian>(color(0.4, 0.9, 0.4));
    auto material_front = std::make_shared<dielectric>(1.5);

    // create world scene
    hittable_list world;
    world.add(std::make_shared<geometry_objects::sphere>  // ground
                      (point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<geometry_objects::sphere>  // center
                      (point3( 0.0,    0.5, -1.0),   0.3, material_center));
    world.add(std::make_shared<geometry_objects::sphere>  // left
                      (point3(-0.6,    0.0, -1.0),   0.5, material_left));
    world.add(std::make_shared<geometry_objects::sphere>  // right
                      (point3( 0.6,    0.0, -1.0),   0.5, material_right));
    world.add(std::make_shared<geometry_objects::sphere>  // behind
                      (point3(0.0, -0.3, -1.5),      0.2, material_behind));
    world.add(std::make_shared<geometry_objects::sphere>  // glass ball
                      (point3(-0.05, 0.0, -0.3),      0.15, material_front));
    world.add(std::make_shared<geometry_objects::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(-0.04, 0.02, -0.3),     -0.01, material_front));
    world.add(std::make_shared<geometry_objects::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(-0.07, -0.04, -0.26),     -0.01, material_front));
    world.add(std::make_shared<geometry_objects::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(0.04, -0.04, -0.33),     -0.01, material_front));

    camera cam;
    cam.vfov = 40;
    cam.lookfrom = point3(-2,2,1);
    cam.lookat   = point3(0,0,-1);
    cam.vup      = vec3(0,1,0);
    cam.set_camera_parameter(16.0 / 9.0, 1600);
    cam.samples_per_pixel = 500;
    cam.max_depth = 50;
    cam.set_output_file("output.ppm");
    cam.initialize();
    cam.set_focus_parameter(10.0, cam.focus_test(world));

    cam.render(world);

    return 0;
}

int fancy_scene() {
    hittable_list world;

    auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<geometry_objects::sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = utilities::random_double();
            point3 center(a + 0.9*utilities::random_double(), 0.2, b + 0.9*utilities::random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random_vec() * color::random_vec();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    world.add(make_shared<geometry_objects::sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random_vec(0.5, 1);
                    auto fuzz = utilities::random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<geometry_objects::sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    world.add(make_shared<geometry_objects::sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(make_shared<geometry_objects::sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<geometry_objects::sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<geometry_objects::sphere>(point3(4, 1, 0), 1.0, material3));

    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 1600);
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.set_output_file("fancy.ppm");
    cam.set_focus_parameter(0.6, 10.0);

    cam.render(world);
}


int main() {
    fancy_scene();
}