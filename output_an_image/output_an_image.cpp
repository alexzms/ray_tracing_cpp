/*
 * This program will output a PPM Image Format
 */

#include "iostream"
#include "../includes/color.h"
#include "../includes/vec3.h"
#include "../includes/ray.h"

int not_use_color_and_vec3() {
    unsigned int image_width = 500;
    unsigned int image_height = 500;

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    // first loop through height, so that the output will be row-by-row
    for (unsigned h = 0; h != image_height; ++h) {
        std::clog << "Lines done: " << h << ' ' << std::flush;
        for (unsigned w = 0; w != image_width; ++w) {
            auto r = (double) w / (image_width - 1);
            auto g = (double) h / (image_height - 1);
//            auto b = (double) (w_ + h) / (image_width + image_width - 2);
            auto b = 0.0;

            int ir = static_cast<int>(r * 255.999);
            int ig = static_cast<int>(g * 255.999);
            int ib = static_cast<int>(b * 255.999);

            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }
    std::cout << std::endl;

    return 0;
}

int main() {
    unsigned int image_width = 500;
    unsigned int image_height = 500;

    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

    // first loop through height, so that the output will be row-by-row
    for (unsigned h = 0; h != image_height; ++h) {
        std::clog << "Lines done: " << h << ' ' << std::flush;
        for (unsigned w = 0; w != image_width; ++w) {
            auto r = (double) w / (image_width - 1);
            auto g = (double) h / (image_height - 1);
//            auto b = (double) (w_ + h) / (image_width + image_width - 2);
            auto b = 0.0;
            color c{r, g, b};
            write_color(std::cout, c);
        }
    }
    std::cout << std::endl;

    return 0;
}