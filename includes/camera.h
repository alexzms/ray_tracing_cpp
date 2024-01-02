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
#include "sstream"
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
        if (stream_is_file && filestream && filestream.is_open()) {
            filestream.close();
            if (filestream.fail()) {
                std::cout << "[deconstruct]Error occurred while closing the file." << std::endl;
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
//            filestream.open(output_file);
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
        load_image(filename, previous_samples);
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

    void arrange_render(const hittable_list& world, unsigned groups, unsigned sample_per_group) {
        if (!initialized) initialize();
        if (output_file.empty()) {
            std::cout << "Error occurred while arranging render. Empty output_file is not supported." << std::endl;
            return;
        }
        if (print_progress) std::cout << "Launching render... Size = " << groups << " x " << sample_per_group << std::endl;
        for (unsigned int i = 0; i != groups; ++i) {
            render_step(world, sample_per_group, true);
        }
    }

    void render(const hittable_list& world, bool empty_file_first = true) {
        if (!initialized) initialize();
        internal_render(world, samples_per_pixel, empty_file_first);
        sample_count += samples_per_pixel;
        if (print_progress) std::clog << "Writing to file..." << std::endl;
        write_all_color(empty_file_first);
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

    double reciprocal_sqrt_spp = 0.0;

    std::unique_ptr<unsigned char[]> image_buffer;                         // store the buffer and count of samples
    unsigned int sample_count = 0;

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

    [[nodiscard]] ray get_ray_defocus_monte_carlo(unsigned int w, unsigned int h, unsigned i, unsigned j) const {
        auto pixel_center = pixel00_location + h * pixel_delta_v + w * pixel_delta_u;
        auto pixel_random = pixel_center + pixel_sample_square_monte_carlo(i, j);

        auto ray_origin = (defocus_angle <= 0) ? camera_center : defocus_disk_sample();
        auto ray_direction = pixel_random - ray_origin;
        auto ray_time = utilities::random_double(0, exposure_time);

        return ray{ray_origin, ray_direction, ray_time};
    }

    [[nodiscard]] point3 pixel_sample_square_monte_carlo(unsigned i, unsigned j) const {
        auto px = -0.5 + (i + utilities::random_double()) * reciprocal_sqrt_spp;
        auto py = -0.5 + (j + utilities::random_double()) * reciprocal_sqrt_spp;      // from -0.5~0.5

        return (px * pixel_delta_u) + (py * pixel_delta_v);                   // multiply the base vector
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
        double pdf = 0.0;
        if (!rec.surface_material->scatter(r, rec, attenuation, scatter_ray, pdf))
            return emission_color;                                                  // no scatter, just emission

                                                                                    // probability of getting scatter_ray
//        double scattering_pdf = rec.surface_material->scattering_pdf(r, rec, scatter_ray);
//        double sample_pdf = scattering_pdf;                                         // relative prob of sampling this ray
//
//        color scatter_color = attenuation
//                * scattering_pdf * ray_color(scatter_ray, remain_depth - 1, world) / sample_pdf;
                                                                                    // naive way
        color scatter_color = attenuation * ray_color(scatter_ray, remain_depth - 1, world);
        return emission_color + scatter_color;
    }

    void load_image(const std::string& filename, unsigned int previous_samples = 1) {
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
            }
        }
        sample_count = previous_samples;
        file.close();
        if (print_progress) std::cout << "Image loaded." << std::endl;
    }

    void render_step(const hittable_list& world, unsigned int samples_per_pixel_step, bool empty_file_first = true) {
        if (print_progress) std::cout << "Rendering... sample steps = " << samples_per_pixel_step << std::endl;
        if (!initialized) initialize();                     // directly call the internal render function
        internal_render(world, samples_per_pixel_step, empty_file_first);
        sample_count += samples_per_pixel_step;
        if (print_progress) std::clog << "Writing to file..., sample count = " << sample_count << std::endl;
        std::string step_string = std::to_string(sample_count);
        // remove .ppm, concatenate the step string, add .ppm
        std::string filename = output_file.substr(0, output_file.size() - 4) + "_" + step_string + ".ppm";
        write_all_color(empty_file_first, filename);
    }

    void internal_render(const hittable_list& world, unsigned int samples = 0, bool empty_file_first = true) {
        if (samples == 0) samples = samples_per_pixel;
        auto sqrt_spp = static_cast<unsigned>(std::sqrt(samples));   // sqrt_spp is the sqrt of samples
        bool use_sqrt = (sqrt_spp * sqrt_spp == samples);               // if samples is a perfect square, use sqrt
        // first loop through height, so that the output will be row-by-row
        if (!use_sqrt) {
            for (unsigned h = 0; h != image_height; ++h) {
                if (print_progress) std::clog << "\rScan lines done: " << h << ' ' << std::flush;
                for (unsigned w = 0; w != image_width; ++w) {
                    color avg_color{0, 0, 0};
                    for (int i = 0; i != samples; ++i) {
                        auto pixel_ray = get_ray_defocus(w, h);
                        avg_color += ray_color(pixel_ray, max_depth, world);   // core render function
                    }
                    buffer_color(avg_color, h, w,samples);
                }
            }
        } else {
            reciprocal_sqrt_spp = 1.0 / sqrt_spp;
            for (unsigned h = 0; h != image_height; ++h) {
                if (print_progress) std::clog << "\rScan lines done: " << h << ' ' << std::flush;
                for (unsigned w = 0; w != image_width; ++w) {
                    color avg_color{0, 0, 0};
                    for (int i = 0; i != sqrt_spp; ++i) {
                        for (int j = 0; j != sqrt_spp; ++j) {
                            auto pixel_ray = get_ray_defocus_monte_carlo(w, h, i, j);
                            avg_color += ray_color(pixel_ray, max_depth, world);   // core render function
                        }
                    }
                    buffer_color(avg_color, h, w, samples);
                }
            }
        }
    }

    void write_all_color(bool empty_file_first, const std::string& force_overwrite_filename = "",
                                                                                    unsigned int depth = 255) {
        if (stream_is_file && !output_file.empty() && !dynamic_cast<std::ofstream*>(output)->is_open()) {
            dynamic_cast<std::ofstream*>(output)->open(output_file);
            if (!dynamic_cast<std::ofstream*>(output)->is_open()) {
                std::cout << "[write_all_color]Error occurred while opening the file." << std::endl;
                return;
            }
        }
        std::ostream *out = output;                                         // general output stream
        std::ofstream force_overwrite_stream;                               // force overwrite stream
        if (!force_overwrite_filename.empty()) {
            force_overwrite_stream.open(force_overwrite_filename);
            if (!force_overwrite_stream.is_open()) {
                std::cout << "[write_all_color]Error occurred while opening the file.(force overwrite file)" << std::endl;
                return;
            }
            out = &force_overwrite_stream;                                  // if force overwrite, use this stream
            if (empty_file_first) {
                force_overwrite_stream.seekp(0);
            }
        } else if (empty_file_first && stream_is_file) {                    // if not force overwrite, use original stream
            if (!dynamic_cast<std::ofstream*>(out)->is_open())
                dynamic_cast<std::ofstream*>(out)->open(output_file);
            dynamic_cast<std::ofstream*>(out)->seekp(0);
        }
        *out << "P3\n" << image_width << ' ' << image_height << "\n" << depth << "\n";
        for (unsigned int h = 0; h != image_height; ++h) {
            for (unsigned int w = 0; w != image_width; ++w) {
                auto buffer_index = (image_height - h - 1) * image_width * 3 + w * 3;
                auto r = static_cast<int>(image_buffer[buffer_index]);          // write int, not char
                auto g = static_cast<int>(image_buffer[buffer_index + 1]);
                auto b = static_cast<int>(image_buffer[buffer_index + 2]);
                *out << r << ' ' << g << ' ' << b << '\n';
            }
        }
        *out << std::endl;                                                      // this will force the stream to write
        if (!force_overwrite_filename.empty()) {                                // if force overwrite, close the stream
            force_overwrite_stream.close();
            return;
        }
        if (force_overwrite_filename.empty() && stream_is_file) {               // if not force overwrite, close the stream
            dynamic_cast<std::ofstream*>(out)->close();
        }
    }

    static inline double linear_to_gamma(double val) {
        return std::sqrt(val);
    }

    void buffer_color(const color &pixel_color, unsigned int h, unsigned int w,
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
                + image_buffer[buffer_index] * sample_count)
                / (sample_count + sample_num);                      // r'=(r_new*new_count+r_old*old_count)/(new+old)
        image_buffer[buffer_index + 1] = (static_cast<unsigned char>(intensity.clamp(g) * coefficient) * sample_num
                + image_buffer[buffer_index + 1] * sample_count)
                / (sample_count + sample_num);
        image_buffer[buffer_index + 2] = (static_cast<unsigned char>(intensity.clamp(b) * coefficient) * sample_num
                + image_buffer[buffer_index + 2] * sample_count)
                / (sample_count + sample_num);
    }

};

#endif //RAY_TRACING_CAMERA_H
