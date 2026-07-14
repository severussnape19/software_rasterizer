module;

#include <limits>
#include <fstream>
#include <vector>

export module display;

import types;

export struct Framebuffer {
public:
    Framebuffer() = default;
    Framebuffer(usize height, usize width)
        : width(width), height(height)
    {
        pixels.resize(width * height * 3);
        depth_buf.resize(width * height, -std::numeric_limits<f32>::infinity());
    }

    auto set_pixel(usize i, usize j, u8 r, u8 g, u8 b, f32 depth) -> void {
        auto index = j * width + i;

        if (depth > depth_buf[index]) {
            depth_buf[index] = depth;
            usize color_idx = index * 3;

            pixels[color_idx] = r;
            pixels[color_idx + 1] = g;
            pixels[color_idx + 2] = b;
        }
    }

    auto save_ppm(std::fstream& file) -> void {
        file << "P3\n" << width << ' ' << height << '\n' << 255 << '\n';
        usize size = pixels.size();
        for (usize i{}; i < size; i += 3) {
            u8 r = pixels[i];
            u8 g = pixels[i + 1];
            u8 b = pixels[i + 2];
            file << +r << ' ' << +g << ' ' << +b << '\n';
        }
    }
public:
    usize width;
    usize height;
    std::vector<u8> pixels;
    std::vector<f32> depth_buf;
};


