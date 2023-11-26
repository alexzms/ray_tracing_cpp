//
// Created by alexzms on 2023/11/20.
//

#ifndef RAY_TRACING_CAMERA_H
#define RAY_TRACING_CAMERA_H

#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "interval.h"
#include "utilities.h"
#include "functional"
#include "fstream"
#include "lambertian.h"

class camera {
public:
    bool print_progress = true;
    unsigned int samples_per_pixel = 10;
    unsigned int max_depth = 50;
    double vfov = 90;
    double exposure_time = 1;
    point3 lookfrom = point3{0, 0, -1};
    point3 lookat = point3{0, 0, 0};
    vec3 vup = vec3{0, 1, 0};

    camera(): output(&std::cout), image_height(0), viewport_width(0.0) {}
    ~camera() {
        if (stream_is_file && filestream) {
            filestream.close();
            if (filestream.fail()) {
                std::cout << "Error occurred while closing the file." << std::endl;
            }
        }
    }

    void set_camera_parameter(double ratio, int width) {
        this->aspect_ratio = ratio;
        this->image_width = width;
    }

    void set_output_file(const std::string &val) {
        this->output_file = val;
    }

    void set_focus_parameter(double angle, double dist = 10.0) {
        this->defocus_angle = angle;
        this->focus_dist = dist;
        if (initialized) {
            refocus();
        }
    }

    [[nodiscard]] double focus_test(const hittable_list& world) {
        if (!initialized) initialize();
        ray test_ray{camera_center, -w_};
        hit_record rec;
        if (world.hit(test_ray, interval(0.0001, utilities::infinity), rec)) {
            return (rec.p - camera_center).length();
        } else {
            return utilities::infinity;
        }
    }

    void initialize() {
        if (initialized) return;
        initialized = true;
        camera_center = lookfrom;
//        focal_length = (lookat - lookfrom).length();
        focal_length = focus_dist;

        w_ = normalize(lookfrom - lookat);                                   // opposite direction to view direction
        u = normalize(cross(vup, w_));
        v = cross(w_, u);

        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height > 1) ? image_height : 1;

        auto theta = utilities::degree_to_radian(vfov);
        auto h = tan(theta / 2);                                     // h is height for unit focal_length
        viewport_height = 2 * h * focal_length;                                 // * focal_length, it's easy to think
        viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

        viewport_u = viewport_width * u;
        viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        viewport_upper_left = camera_center - (focal_length * w_) - viewport_u / 2 - viewport_v / 2;
        pixel00_location = viewport_upper_left + 0.5 * pixel_delta_u + 0.5 * pixel_delta_v;

        auto defocus_radius = focus_dist * tan(utilities::degree_to_radian(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;                                    // camera defocus plane, basis vector
        defocus_disk_v = v * defocus_radius;
        if (!output_file.empty() && !filestream.is_open()) {
            std::cout << "Camera init: output file is not empty, switching to file stream.." << std::endl;
            filestream.open(output_file);
            output = &filestream;
            stream_is_file = true;
        }
    }

    void render(const hittable_list& world, bool empty_file_first = true) {
        if (!initialized) initialize();
        if (empty_file_first && stream_is_file) {
            dynamic_cast<std::ofstream*>(output)->seekp(0);
        }
        *output << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        // first loop through height, so that the output will be row-by-row
        for (unsigned h = 0; h != image_height; ++h) {
            if (print_progress) std::clog << "\rScan lines done: " << h << ' ' << std::flush;
            for (unsigned w = 0; w != image_width; ++w) {
                color avg_color{0, 0, 0};
                for (int i = 0; i != samples_per_pixel; ++i) {
                    auto pixel_ray = get_ray_defocus(w, h);
                    avg_color += ray_color(pixel_ray, max_depth, world);              // core render function
                }
                write_color(*output, avg_color, samples_per_pixel);
            }
        }
        *output << std::endl;                                                 // this will force the stream to write
    }


private:
    double aspect_ratio = 16.0/9.0;                                        // default value is 1600x900
    int image_width = 1600;
    int image_height;                                                      // we need not read this before initialize()
    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width;                                                 // we need not read this before initialize()
    point3 camera_center = point3{0, 0, 0};
    std::ostream *output;                                                  // if we want to output to file, set the
    std::string output_file;                                               // output_file to some string

    vec3 u, v, w_;                                                          // camera coordinate basis

    vec3 defocus_disk_u;                                                   // camera defocus plane
    vec3 defocus_disk_v;

    double defocus_angle = 0;
    double focus_dist = 10;

    vec3 viewport_u;
    vec3 viewport_v;

    vec3 pixel_delta_u;
    vec3 pixel_delta_v;                                                    // the base vector, expands a linear space

    vec3 viewport_upper_left;
    vec3 pixel00_location;

    std::ofstream filestream;
    bool stream_is_file = false;

    bool initialized = false;

    void refocus() {
        if (!initialized) initialize();
        focal_length = focus_dist;
        auto theta = utilities::degree_to_radian(vfov);
        auto h = tan(theta / 2);
        viewport_height = 2 * h * focal_length;
        viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);
        viewport_u = viewport_width * u;
        viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        viewport_upper_left = camera_center - (focal_length * w_) - viewport_u / 2 - viewport_v / 2;
        pixel00_location = viewport_upper_left + 0.5 * pixel_delta_u + 0.5 * pixel_delta_v;

        auto defocus_radius = focus_dist * tan(utilities::degree_to_radian(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;                                    // camera defocus plane, basis vector
        defocus_disk_v = v * defocus_radius;
    }

    [[nodiscard]] ray get_ray_defocus(unsigned int w, unsigned int h) const {
        auto pixel_center = pixel00_location + h * pixel_delta_v + w * pixel_delta_u;
        auto pixel_random = pixel_center + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? camera_center : defocus_disk_sample();
        auto ray_direction = pixel_random - ray_origin;
        auto ray_time = utilities::random_double(0, exposure_time);

        return ray{ray_origin, ray_direction, ray_time};
    }

    [[nodiscard]] vec3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return camera_center + p[0] * defocus_disk_u + p[1] * defocus_disk_v;
    }


    [[nodiscard]] ray get_ray(unsigned int w, unsigned int h) const {
        auto pixel_center = pixel00_location + h * pixel_delta_v + w * pixel_delta_u;
        auto pixel_random = pixel_center + pixel_sample_square();

        auto ray_origin = camera_center;
        auto ray_direction = pixel_random - ray_origin;
        auto ray_time = utilities::random_double(0, exposure_time);

        return ray{ray_origin, ray_direction, ray_time};
    }

    [[nodiscard]] point3 pixel_sample_square() const {
        auto px = utilities::random_double(-0.5, 0.5);
        auto py = utilities::random_double(-0.5, 0.5);      // from -0.5~0.5

        return (px * pixel_delta_u) + (py * pixel_delta_v);                   // multiply the base vector
    }


    [[nodiscard]] static color ray_color(const ray &r, unsigned int remain_depth, const hittable& world) {
        if (remain_depth <= 0) return color{0, 0, 0};
        hit_record rec;
        if (world.hit(r, interval(0.0001, utilities::infinity), rec)) {
            ray scatter_ray;
            color attenuation;
            if (rec.surface_material->scatter(r, rec, attenuation, scatter_ray)) {
                return attenuation * ray_color(scatter_ray, remain_depth - 1, world);
            }
            return color{0, 0, 0};
        }
        auto blend_factor = 0.5 * (r.direction().y() + 1.0);
        color color1{1.0, 1.0, 1.0};
        color color2{0.5, 0.7, 1.0};
        return (1 - blend_factor) * color1 + blend_factor * color2;
    }

};

#endif //RAY_TRACING_CAMERA_H
