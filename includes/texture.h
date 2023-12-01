//
// Created by alexzms on 2023/11/25.
//

#ifndef RAY_TRACING_TEXTURE_H
#define RAY_TRACING_TEXTURE_H

#include <utility>
#include "vec3.h"
#include "color.h"
#include "utilities.h"
#include "image_utils.h"
#include "perlin.h"

namespace texture {
    class texture_base {
    public:
        virtual ~texture_base() = default;
        [[nodiscard]] virtual color value(double u, double v, const point3 &p) const = 0;
    };

    class solid_color: public texture_base {
    public:
        explicit solid_color(color val): c(std::move(val)) {}
        solid_color(double r, double g, double b): c(r, g, b) {}
        [[nodiscard]] color value(double u, double v, const point3 &p) const override {
            return c;
        }

    private:
        color c;
    };

    class checker_texture: public texture_base {
    public:
        checker_texture(double scale, std::shared_ptr<texture_base> odd_tex, std::shared_ptr<texture_base> even_tex):  // pass-in by
                inv_scale(1.0 / scale), odd_tex(std::move(odd_tex)), even_tex(std::move(even_tex)) {}            // value and move
        checker_texture(double scale, const color &c1, const color &c2): inv_scale(1.0 / scale),
                                                                         odd_tex(std::make_shared<solid_color>(c1)), even_tex(std::make_shared<solid_color>(c2)) {}

        [[nodiscard]] color value(double u, double v, const point3 &p) const override {
            auto x = static_cast<int>(std::floor(p.x() * inv_scale));                          // floor for consistency
            auto y = static_cast<int>(std::floor(p.y() * inv_scale));
            auto z = static_cast<int>(std::floor(p.z() * inv_scale));

            bool is_even = ((x + y + z) % 2) == 0;
            return is_even ? even_tex->value(u, v, p) : odd_tex->value(u, v, p);
        }
    private:
        double inv_scale;
        std::shared_ptr<texture_base> odd_tex;
        std::shared_ptr<texture_base> even_tex;
    };

    class image_texture: public texture_base {
    public:
        explicit image_texture(std::shared_ptr<image_object> img): img(std::move(img)) {}   // construct by obj or by name
        explicit image_texture(const char *filename): img(std::make_shared<image_object>(filename)) {}

        [[nodiscard]] color value(double u, double v, const point3 &p) const override {
            if (img->height() <= 0) return {0, 1, 1};               // If the image is not loaded correctly, use
            // solid cyan for debugging(universal rule).
            u = interval(0, 1).clamp(u);                        // Clamp u, v to valid range. no need to flip
            v = 1.0 - interval(0, 1).clamp(v);                        // v, because we did that in image_utils.

            auto x_img = static_cast<int>(u * img->width());
            auto y_img = static_cast<int>(v * img->height());
            auto pixel_color = img->pixel_data(x_img, y_img);

            return {rgb_scale * pixel_color[0], rgb_scale * pixel_color[1], rgb_scale * pixel_color[2]};
        }
    private:
        std::shared_ptr<image_object> img;
        double rgb_scale = 1.0 / 255.0;                                         // 8-bit image, used most frequently
    };

    class noise_texture: public texture_base {
    public:
        noise_texture(): noise(std::make_shared<perlin>()), freq(1.0) {}
        noise_texture(double freq): noise(std::make_shared<perlin>()), freq(freq) {}
        [[nodiscard]] color value(double u, double v, const point3 &p) const override {
            auto s = freq * p;
            return color{1, 1, 1} * 0.5 * (1 + sin(s.z() + 10*noise->turbulence(s)));
        }
    private:
        std::shared_ptr<perlin> noise;
        double freq;
    };
}

#endif //RAY_TRACING_TEXTURE_H
