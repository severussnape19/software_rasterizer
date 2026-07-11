#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <vector>

import math;
import types;

struct Framebuffer {
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
        auto size = pixels.size();
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

auto edge_function(Vec3<f32> const& a, Vec3<f32> const& b, Vec3<f32> const& p) -> f32 {
    auto ba = Vec2<f32>(b.x - a.x, b.y - a.y);
    auto pa = Vec2<f32>(p.x - a.x, p.y - a.y);
    return ba.perp_dot(pa);
}

auto draw_triangle(
    Framebuffer& fb,
    Vec3<f32> const& a,
    Vec3<f32> const& b,
    Vec3<f32> const& c,
    u8 r, u8 g, u8 b_col
) -> void {

    // bounding box co-ords
    auto bb_min = Vec3<f32>(
        std::min(std::min(a.x, b.x), c.x),
        std::min(std::min(a.y, b.y), c.y),
        0.f
    );

    auto bb_max = Vec3<f32>(
        std::max(std::max(a.x, b.x), c.x),
        std::max(std::max(a.y, b.y), c.y),
        0.f
    );

    usize min_x = static_cast<usize>(
        std::clamp(std::floor(bb_min.x), 0.f, static_cast<f32>(fb.width) - 1.f)
    );
    usize min_y = static_cast<usize>(
        std::clamp(std::floor(bb_min.y), 0.f, static_cast<f32>(fb.height) - 1.f)
    );

    usize max_x = static_cast<usize>(
        std::clamp(std::ceil(bb_max.x), 0.f, static_cast<f32>(fb.width) - 1.f)
    );
    usize max_y = static_cast<usize>(
        std::clamp(std::ceil(bb_max.y), 0.f, static_cast<f32>(fb.height) - 1.f)
    );

    for (usize i{min_x}; i < max_x; ++i) {
        for (usize j{min_y}; j < max_y; ++j) {
            auto p = Vec3<f32>(i, j);

            f32 w0 = edge_function(a, b, p);
            f32 w1 = edge_function(b, c, p);
            f32 w2 = edge_function(c, a, p);

            if (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f) {
                f32 total = w0 + w1 + w2;

                f32 alpha = w1 / total;
                f32 beta  = w2 / total;
                f32 gamma = w0 / total;

                f32 depth = alpha * a.z + beta * b.z + gamma * c.z;

                fb.set_pixel(i, j, r, g, b_col, depth);
            }
        }
    }
}

auto project(Vec3<f32> v, usize width, usize height) -> Vec3<f32> {
    f32 w = -v.z;
    f32 ndc_x = v.x / w;
    f32 ndc_y = v.y / w;

    auto pixel_scale = Vec3<f32>(
       ((ndc_x + 1) * 0.5) * (width - 1),
       ((1 - ndc_y) * 0.5) * (height - 1),
       w
    );

    return pixel_scale;
}

auto main(i32 argc, char* argv[]) -> i32 {
    if (argc < 2) {
        std::cerr << "[ERR] File where bish\n";
        return EXIT_FAILURE;
    }
    constexpr usize WIDTH = 800;
    constexpr usize HEIGHT = 600;

    auto framebuffer = Framebuffer(HEIGHT, WIDTH);

    std::fstream outfile(argv[1], std::ios::out);
    if (!outfile.is_open()) {
        std::cerr << "could not open file!\n";
        return EXIT_FAILURE;
    }

    auto a = project(Vec3<f32>(-0.5, 0.5, -1.0), WIDTH, HEIGHT);
    auto b = project(Vec3<f32>(0.5, 0.5, -2.0), WIDTH, HEIGHT);
    auto c = project(Vec3<f32>(0.0, -0.5, -1.5), WIDTH, HEIGHT);
    draw_triangle(framebuffer, a, b, c, 255, 0, 0);
    framebuffer.save_ppm(outfile);
    outfile.close();
    return EXIT_SUCCESS;
}
