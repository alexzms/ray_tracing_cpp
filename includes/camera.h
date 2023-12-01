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
    bool print_progress = true;                            // print the "lines done: xx"
    unsigned int samples_per_pixel = 10;                   // pixel sample time, large for better visual effects
    unsigned int max_depth = 50;                           // light ray bounce max depth
    double vfov = 90;                                      // fov_height, width  will be calculated based on ratio
    double exposure_time = 1;                              // motion blur exposure time
    point3 lookfrom = point3{0, 0, -1};        // look from position
    point3 lookat = point3{0, 0, 0};           // look at position
    vec3 vup = vec3{0, 1, 0};                  // camera vup vector, actually it controls the rotation
    std::function<color(double)> background_function =     // function controls how the background color will be rendered
            [](double blend_factor) -> color {
                color color1{1.0, 1.0, 1.0};
                color color2{0.5, 0.7, 1.0};
                return (1 - blend_factor) * color1 + blend_factor * color2;
            };

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
        if (filestream.is_open())
            filestream.close();

        if (!output_file.empty() && !filestream.is_open()) {
            std::cout << "Camera init: output file is not empty, switching to file stream.." << std::endl;
            filestream.open(output_file);
            output = &filestream;
            stream_is_file = true;
        }
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

    void set_prev_image(const std::string& filename, unsigned int previous_samples = 1) {
        if (!initialized) initialize();
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error occurred while opening the file." << std::endl;
            return;
        }
        std::string line;
        // read the first line, should be "P3"
        std::getline(file, line);
        if (line != "P3") {
            std::cout << "Error occurred while reading the file. The file is not a PPM file." << std::endl;
            return;
        }
        // read the second line, should be image_width image_height
        std::getline(file, line);
        std::stringstream ss(line);
        int width, height;
        ss >> width >> height;
        if (width != image_width || height != image_height) {
            std::cout << "Error occurred while reading the file. The file's size does not match." << std::endl;
            return;
        }
        // read the third line, should be 255
        std::getline(file, line);
        if (line != "255") {
            std::cout << "Error occurred while reading the file. The file's color depth is not 255." << std::endl;
            return;
        }
        // read the rest of the file
        for (unsigned int h = 0; h != image_height; ++h) {
            for (unsigned int w = 0; w != image_width; ++w) {
                auto buffer_index = (image_height - h - 1) * image_width * 3 + w * 3;
                int r, g, b;
                file >> r >> g >> b;
                image_buffer[buffer_index] = r;
                image_buffer[buffer_index + 1] = g;
                image_buffer[buffer_index + 2] = b;
                sample_count[h * image_width + w] = previous_samples;
            }
        }
        file.close();
        std::cout << "Image loaded." << std::endl;
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
                                                                                 // late initialize the buffer
                                                                                 // they are all zero-initialized
        image_buffer = std::make_unique<unsigned char[]>(image_width * image_height * 3);
        sample_count = std::make_unique<unsigned int[]>(image_width * image_height);

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
//        if (!output_file.empty() && !filestream.is_open()) {
//            std::cout << "Camera init: output file is not empty, switching to file stream.." << std::endl;
//            filestream.open(output_file);
//            output = &filestream;
//            stream_is_file = true;
//        }
    }

    void render(const hittable_list& world, bool empty_file_first = true) {
        if (!initialized) initialize();
        // first loop through height, so that the output will be row-by-row
        for (unsigned h = 0; h != image_height; ++h) {
            if (print_progress) std::clog << "\rScan lines done: " << h << ' ' << std::flush;
            for (unsigned w = 0; w != image_width; ++w) {
                color avg_color{0, 0, 0};
                for (int i = 0; i != samples_per_pixel; ++i) {
                    auto pixel_ray = get_ray_defocus(w, h);
                    avg_color += ray_color(pixel_ray, max_depth, world);   // core render function
                }
                buffer_color(*output,avg_color, h, w,samples_per_pixel);
            }
        }
        if (print_progress) std::clog << "Writing to file..." << std::endl;
        write_all_color(*output, empty_file_first);
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

    std::unique_ptr<unsigned char[]> image_buffer;                         // store the buffer and count of samples
    std::unique_ptr<unsigned int[]>  sample_count;

    vec3 u, v, w_;                                                         // camera coordinate basis

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


    [[nodiscard]] color ray_color(const ray &r, unsigned int remain_depth, const hittable& world) {
        if (remain_depth <= 0) return color{0, 0, 0};                  // exceeds depths limit

        hit_record rec;
        auto blend_factor = 0.5 * (r.direction().y() + 1.0);
        if (!world.hit(r, interval(0.0001, utilities::infinity), rec))
            return background_function(blend_factor);                               // no hit -> return background color

        color emission_color = rec.surface_material->emitted(rec.u, rec.v, rec.p);  // emission term
        ray scatter_ray;                                                            // scatter term
        color attenuation;
        if (!rec.surface_material->scatter(r, rec, attenuation, scatter_ray))
            return emission_color;                                                  // no scatter hit, just emission

        color scatter_color = attenuation * ray_color(scatter_ray, remain_depth - 1, world);
        return emission_color + scatter_color;
    }

    void write_all_color(std::ostream &out, bool empty_file_first, unsigned int depth = 255) {
        if (empty_file_first && stream_is_file) {
            dynamic_cast<std::ofstream*>(output)->seekp(0);
        }
        out << "P3\n" << image_width << ' ' << image_height << "\n" << depth << "\n";
        for (unsigned int h = 0; h != image_height; ++h) {
            for (unsigned int w = 0; w != image_width; ++w) {
                auto buffer_index = (image_height - h - 1) * image_width * 3 + w * 3;
                auto r = static_cast<int>(image_buffer[buffer_index]);          // write int, not char
                auto g = static_cast<int>(image_buffer[buffer_index + 1]);
                auto b = static_cast<int>(image_buffer[buffer_index + 2]);
                out << r << ' ' << g << ' ' << b << '\n';
            }
        }
        out << std::endl;                                                       // this will force the stream to write
    }

    static inline double linear_to_gamma(double val) {
        return std::sqrt(val);
    }

    void buffer_color(std::ostream &out, const color &pixel_color, unsigned int h, unsigned int w,
                                                    unsigned int sample_num, unsigned int depth = 255) {
        static const interval intensity(0, 0.999);
        auto r = pixel_color.x();
        auto g = pixel_color.y();
        auto b = pixel_color.z();

        // Divide the color by the number of samples and gamma-correct for gamma=2.0.
        auto scale = 1.0 / sample_num;
        r = linear_to_gamma(r * scale);
        g = linear_to_gamma(g * scale);
        b = linear_to_gamma(b * scale);

        // Write the translated [0,255] value of each color component.
        double coefficient = depth + 0.999;
        // write to buffer
        auto buffer_index = (image_height - h - 1) * image_width * 3 + w * 3;
        auto buffer2_index = h * image_width + w;          // merge old and new
        image_buffer[buffer_index] = (static_cast<unsigned char>(intensity.clamp(r) * coefficient) * sample_num
                + image_buffer[buffer_index] * sample_count[buffer2_index])
                / (sample_count[h * image_width + w] + sample_num);    // r'=(r_new*new_count+r_old*old_count)/(new+old)
        image_buffer[buffer_index + 1] = (static_cast<unsigned char>(intensity.clamp(g) * coefficient) * sample_num
                + image_buffer[buffer_index + 1] * sample_count[buffer2_index])
                / (sample_count[h * image_width + w] + sample_num);
        image_buffer[buffer_index + 2] = (static_cast<unsigned char>(intensity.clamp(b) * coefficient) * sample_num
                + image_buffer[buffer_index + 2] * sample_count[buffer2_index])
                / (sample_count[h * image_width + w] + sample_num);
        // update sample count
        sample_count[h * image_width + w] += sample_num;
    }

};

#endif //RAY_TRACING_CAMERA_H
