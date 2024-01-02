/*
 * This program will output a PPM Image Format
 */

#include "iostream"
#include "chrono"
#include "./includes/common.h"


//double hit_sphere(const ray& r, const point3& sphere_center, const double& radius) {
//    // a = dir . dir = ||dir||_2^2 = 1, b = 2 * dir . (origin-center1), c = ||origin- center1||_2^2 - radius^2
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
    auto material_ground = std::make_shared<material::lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<material::lambertian>(color(0.7, 0.3, 0.3));
    auto material_left   = std::make_shared<material::lambertian>(color(0.9, 0.9, 0.9));
    auto material_right  = std::make_shared<material::metal>(color(0.95, 0.95, 0.95), 0.0);
    auto material_behind = std::make_shared<material::lambertian>(color(0.4, 0.9, 0.4));
    auto material_front = std::make_shared<material::dielectric>(1.5);

    // create world scene
    hittable_list world;
    world.add(std::make_shared<primitive::sphere>  // ground
                      (point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<primitive::sphere>  // center1
                      (point3( 0.0,    0.5, -1.0),   0.3, material_center));
    world.add(std::make_shared<primitive::sphere>  // left
                      (point3(-0.6,    0.0, -1.0),   0.5, material_left));
    world.add(std::make_shared<primitive::sphere>  // right
                      (point3( 0.6,    0.0, -1.0),   0.5, material_right));
    world.add(std::make_shared<primitive::sphere>  // behind
                      (point3(0.0, -0.3, -1.5),      0.2, material_behind));
    world.add(std::make_shared<primitive::sphere>  // glass ball
                      (point3(-0.05, 0.0, -0.3),      0.15, material_front));
    world.add(std::make_shared<primitive::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(-0.04, 0.02, -0.3),     -0.01, material_front));
    world.add(std::make_shared<primitive::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(-0.07, -0.04, -0.26),     -0.01, material_front));
    world.add(std::make_shared<primitive::sphere>  // hollow glass ball inner side, notice r < 0
                      (point3(0.04, -0.04, -0.33),     -0.01, material_front));

    camera cam;
    cam.vfov = 40;
    cam.lookfrom = point3(-2,2,1);
    cam.lookat   = point3(0,0,-1);
    cam.vup      = vec3(0,1,0);
    cam.set_camera_parameter(16.0 / 9.0, 1600);
    cam.samples_per_pixel = 500;
    cam.max_depth = 50;
    cam.set_output_file("output/output.ppm");
    cam.initialize();
    cam.set_focus_parameter(10.0, cam.focus_test(world));

    cam.render(world);

    return 0;
}

int fancy_scene() {
    hittable_list world;

    auto checker = std::make_shared<texture::checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    world.add(std::make_shared<primitive::sphere>(point3(0, -1000, 0), 1000, std::make_shared<material::lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = utilities::random_double();
            point3 center(a + 0.9*utilities::random_double(), 0.2, b + 0.9*utilities::random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material::material_base> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random_vec() * color::random_vec();
                    sphere_material = std::make_shared<material::lambertian>(albedo);
//                    auto center2 = center + vec3(0, utilities::random_double(0,.5), 0);
//                    world.add(make_shared<primitive::sphere>(center, center2, 0.2, sphere_material));
                    world.add(make_shared<primitive::sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random_vec(0.5, 1);
                    auto fuzz = utilities::random_double(0, 0.5);
                    sphere_material = std::make_shared<material::metal>(albedo, fuzz);
                    world.add(make_shared<primitive::sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<material::dielectric>(1.5);
                    world.add(make_shared<primitive::sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<material::dielectric>(1.5);
    world.add(make_shared<primitive::sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<material::lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<primitive::sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<material::metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<primitive::sphere>(point3(4, 1, 0), 1.0, material3));

    world = hittable_list(std::make_shared<bvh_node>(world));            // use bvh_node to accelerate(up tp 10x)

    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 400);
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;
    cam.exposure_time = 0.0;                                                    // static scene

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.set_output_file("output/fancy.ppm");
    cam.set_focus_parameter(0.6, 10.0);

    cam.render(world);
    return 0;
}


int two_spheres() {
    hittable_list world;

    auto checker = std::make_shared<texture::checker_texture>(0.8, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<primitive::sphere>(point3(0, -10, 0), 10, std::make_shared<material::lambertian>(checker)));
    world.add(make_shared<primitive::sphere>(point3(0, 10, 0), 10, std::make_shared<material::lambertian>(checker)));

    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 400);
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;
    cam.exposure_time = 0.0;                                                    // static scene

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);
    cam.set_output_file("fancy.ppm");
    cam.set_focus_parameter(0.6, 10.0);

    cam.render(world);
    return 0;
}

void earth() {
    auto earth_texture = std::make_shared<texture::image_texture>("earthmap.jpg");
    auto earth_surface = std::make_shared<material::lambertian>(earth_texture);
    auto globe = std::make_shared<primitive::sphere>
            (point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 400);
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 30;
    cam.lookfrom = point3(0,0,12);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.set_output_file("output/earth.ppm");
//    cam.initialize();
//    cam.set_focus_parameter(0.0, cam.focus_test(hittable_list(globe)));

    cam.render(hittable_list(globe));
}

int two_perlin_spheres() {
    hittable_list world;

    auto pertext = std::make_shared<texture::noise_texture>(4);
    world.add(make_shared<primitive::sphere>(point3(0, -1000, 0), 1000, make_shared<material::lambertian>(pertext)));
    world.add(make_shared<primitive::sphere>(point3(0, 2, 0), 2, make_shared<material::lambertian>(pertext)));
    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 400);
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.set_output_file("output/perlin.ppm");
    cam.set_focus_parameter(0);

    cam.render(world);
    return 0;
}

void quads() {
    hittable_list world;

    // Materials
    auto left_red     = std::make_shared<material::lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = std::make_shared<material::lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = std::make_shared<material::lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = std::make_shared<material::lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal   = std::make_shared<material::lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<primitive::quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<primitive::quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<primitive::quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<primitive::quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<primitive::quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.set_camera_parameter(1.0, 400);
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.set_output_file("output/quad.ppm");
    cam.set_focus_parameter(0.0, 10.0);

    cam.render(world);
}

void simple_light() {
    hittable_list world;

    auto pertext = std::make_shared<texture::noise_texture>(4);
    world.add(make_shared<primitive::sphere>(point3(0,-1000,0), 1000, make_shared<material::lambertian>(pertext)));
    world.add(make_shared<primitive::sphere>(point3(0,2,0), 2, make_shared<material::lambertian>(pertext)));

    // note the color is brighter than (1, 1, 1) so that it can light up other thingss
    auto difflight = std::make_shared<material::diffuse_light>(color(4,4,4));
    world.add(std::make_shared<primitive::quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));
    world.add(make_shared<primitive::sphere>(point3(0,7,0), 2, difflight));

    camera cam;

    cam.set_camera_parameter(16.0 / 9.0, 800);
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;
    cam.background_function = [](double blend_factor) -> color {
        return color{0, 0, 0};
    };                                                                  // dark black night...

    cam.vfov     = 20;
    cam.lookfrom = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.set_focus_parameter(0.0);
    cam.set_output_file("output/simple_light.ppm");

    cam.render(world);
}

void cornell_box() {
    hittable_list world;

    auto red   = std::make_shared<material::lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<material::lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<material::lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<material::diffuse_light>(color(15, 15, 15));

    world.add(make_shared<primitive::quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<primitive::quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<primitive::quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<primitive::quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<primitive::quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<primitive::quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = instance::box(point3(0,0,0), point3(165,330,165), white);
    box1 = std::make_shared<instance::rotate_y>(box1, 15);
    box1 = std::make_shared<instance::translate>(box1, vec3(265,0,295));
    world.add(box1);

    std::shared_ptr<hittable> box2 = instance::box(point3(0,0,0), point3(165,165,165), white);
    box2 = std::make_shared<instance::rotate_y>(box2, -18);
    box2 = std::make_shared<instance::translate>(box2, vec3(130,0,65));
    world.add(box2);

    world = hittable_list(std::make_shared<bvh_node>(world));

    camera cam;

    cam.set_camera_parameter(1.0, 600);
    cam.samples_per_pixel = 1500;
    cam.max_depth         = 100;
    cam.background_function = [](double _) -> color { return color{0, 0, 0}; };

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.set_output_file("output/cornell.ppm");
    cam.set_focus_parameter(0.0);

    cam.render(world);
}

void cornell_smoke() {
    hittable_list world;

    auto red   = std::make_shared<material::lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<material::lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<material::lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<material::diffuse_light>(color(7, 7, 7));

    world.add(make_shared<primitive::quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<primitive::quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<primitive::quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    world.add(make_shared<primitive::quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<primitive::quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<primitive::quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    std::shared_ptr<hittable> box1 = instance::box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<instance::rotate_y>(box1, 15);
    box1 = make_shared<instance::translate>(box1, vec3(265,0,295));

    std::shared_ptr<hittable> box2 = instance::box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<instance::rotate_y>(box2, -18);
    box2 = make_shared<instance::translate>(box2, vec3(130,0,65));

    world.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    camera cam;

    cam.set_camera_parameter(1.0, 600);
    cam.samples_per_pixel = 50;
    cam.max_depth         = 50;
    cam.background_function = [](double _) -> color { return color{0, 0, 0}; };

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);
    cam.initialize();
//    cam.set_prev_image("output/cornell_smoke.ppm", 151);
    cam.set_output_file("output/cornell_smoke.ppm");

    cam.set_focus_parameter(0.0);

    cam.render(world);
}

void final_scene() {
    hittable_list boxes1;
    auto ground = std::make_shared<material::lambertian>(color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = 1 + sin(i + j) * 101;
            auto z1 = z0 + w;

            boxes1.add(instance::box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list world;

    world.add(std::make_shared<bvh_node>(boxes1));

    auto light = std::make_shared<material::diffuse_light>(color(7, 7, 7));
    world.add(make_shared<primitive::quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto sphere_material = std::make_shared<material::lambertian>(color(0.7, 0.3, 0.1));
    world.add(make_shared<primitive::sphere>(center1, center2, 50, sphere_material));

    world.add(make_shared<primitive::sphere>(point3(260, 150, 45), 50, std::make_shared<material::dielectric>(1.5)));
    world.add(make_shared<primitive::sphere>(
            point3(0, 150, 145), 50, std::make_shared<material::metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = std::make_shared<primitive::sphere>(point3(360,150,145), 70, std::make_shared<material::dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<primitive::sphere>(point3(0,0,0), 5000, std::make_shared<material::dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = make_shared<material::lambertian>(std::make_shared<texture::image_texture>("earthmap.jpg"));
    world.add(make_shared<primitive::sphere>(point3(400,200,400), 100, emat));
    auto pertext = std::make_shared<texture::noise_texture>(0.1);
    world.add(make_shared<primitive::sphere>(point3(220,280,300), 80, make_shared<material::lambertian>(pertext)));

    hittable_list boxes2;
    auto white = std::make_shared<material::lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        auto linear_arrange_center = normalize(vec3(sin(j), cos(j), tan(j))) * 165.0;
        boxes2.add(make_shared<primitive::sphere>(linear_arrange_center, 10, white));
    }

    world.add(make_shared<instance:: translate>(
                      make_shared<instance::rotate_y>(
                              std::make_shared<bvh_node>(boxes2), 15),
                      vec3(-100,270,395)
              )
    );

    camera cam;

    cam.set_camera_parameter(1.0, 800);
    cam.samples_per_pixel = 50;
    cam.max_depth         = 40;
    cam.background_function = [](double _) -> color { return color{0, 0, 0}; };

    cam.vfov     = 40;
    cam.lookfrom = point3(478, 278, -600);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.initialize();

    cam.set_output_file("output/final/final.ppm");
    cam.set_focus_parameter(0.0);

//    cam.render(world);
    cam.set_prev_image("output/final/final_3260.ppm", 3260);
    cam.arrange_render(world, 10, 500);
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    switch(8) {
        case 0: sample_scene(); break;
        case 1: fancy_scene(); break;
        case 2: two_spheres(); break;
        case 3: earth(); break;
        case 4: two_perlin_spheres(); break;
        case 5: quads(); break;
        case 6: simple_light(); break;
        case 7: cornell_box(); break;
        case 8: cornell_smoke(); break;
        case 9: final_scene(); break;
        default: sample_scene();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed(s): " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << std::endl;
    return 0;
}