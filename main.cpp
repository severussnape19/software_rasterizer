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

auto edge_function(Vec4<f32> const& a, Vec4<f32> const& b, Vec4<f32> const& p) -> f32 {
    auto ba = Vec2<f32>(b.x - a.x, b.y - a.y);
    auto pa = Vec2<f32>(p.x - a.x, p.y - a.y);
    return pa.perp_dot(ba);
}

auto draw_triangle(
    Framebuffer& fb,
    Vec4<f32> const& a,
    Vec4<f32> const& b,
    Vec4<f32> const& c,
    u8 r, u8 g, u8 b_col,
    f32 brightness
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

    // Clamp to min and max vals to stay within valid interval
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

    // Iter through every pixel in the triangle
    for (usize i{min_x}; i < max_x; ++i) {
        for (usize j{min_y}; j < max_y; ++j) {
            auto p = Vec3<f32>(i, j);

            f32 w0 = edge_function(a, b, p);
            f32 w1 = edge_function(b, c, p);
            f32 w2 = edge_function(c, a, p);

            // If each of the w vals are greater than or equal to 0, they exist on the edge or inside.
            if (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f) {
                f32 total = w0 + w1 + w2;

                // Barycentric co-ords
                f32 alpha = w1 / total;
                f32 beta  = w2 / total;
                f32 gamma = w0 / total;

                // Distant pixels have greater depth.
                // greater -ve values imply more distance from the camera.
                f32 depth = alpha * a.z + beta * b.z + gamma * c.z;

                u8 ir = static_cast<u8>(r * brightness);
                u8 ig = static_cast<u8>(g * brightness);
                u8 ib = static_cast<u8>(b_col * brightness);

                fb.set_pixel(i, j, ir, ig, ib, depth);
            }
        }
    }
}

auto project(Vec4<f32> v, usize width, usize height) -> Vec3<f32> {
    // Project world space co-ordinates into 2D projected co-ordinates
    auto ndc_x = Vec4<f32>(v.x, 0.f, 0.f, 0.f);
    auto ndc_y = Vec4<f32>(0.f, v.y, 0.f, 0.f);
    auto ndc_z = Vec4<f32>(0.f, 0.f, 1.f, 0.f);
    auto ndc_w = Vec4<f32>(0.f, 0.f, -1.f, 0.f);

    auto pixel_scale = Vec3<f32>(
       ((ndc_x.x + 1) * 0.5) * (width - 1),
       ((1 - ndc_y.y) * 0.5) * (height - 1), // since the y axis is flipped we use this
       v.z
    );

    return pixel_scale;
}

auto main(i32 argc, char* argv[]) -> i32 {
    if (argc < 2) {
        std::cerr << "[ERR] File path where bish?\n";
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

    // Light direction
    Vec3<f32> light_dir = Vec3<f32>(0.f, 0.f, 1.f).normalized();

    // Worldspace-coordinates
    // These are located with respect to the world's origin
    auto ws_a = Vec4<f32>(-0.5f,  0.5f, -1.0f, 1.f);
    auto ws_b = Vec4<f32>( 0.0f, -0.5f, -1.5f, 1.f);
    auto ws_c = Vec4<f32>( 0.5f,  0.5f, -2.0f, 1.f);

    auto model_matrix = Mat4<f32>::translation_matrix(0.5f, 0.f, 0.f);
    auto view_matrix  = Mat4<f32>::identity_matrix();
    auto projection_matrix = Mat4<f32>::projection_matrix();

    std::cout << "World space co-ordinates Before: \n";
    std::cout << "a: " << ws_a << '\n';
    std::cout << "b: " << ws_b << '\n';
    std::cout << "c: " << ws_c << '\n';

    // world space to camera space
    Mat4<f32> mvp = projection_matrix * view_matrix * model_matrix;
    Vec4<f32> clip_a = mvp * ws_a;
    Vec4<f32> clip_b = mvp * ws_b;
    Vec4<f32> clip_c = mvp * ws_c;

    std::cout << "World space co-ordinates After (clip co-ords): \n";
    std::cout << "a: " << clip_a << '\n';
    std::cout << "b: " << clip_b << '\n';
    std::cout << "c: " << clip_c << '\n';

    // Perspective divide
    clip_a /= clip_a.w;
    clip_b /= clip_b.w;
    clip_c /= clip_c.w;

    std::cout << "Perspective divide: \n";
    std::cout << "a: " << clip_a << '\n';
    std::cout << "b: " << clip_b << '\n';
    std::cout << "c: " << clip_c << '\n';

    // Pixel space conversion
    auto a = Vec4<f32>(
        ((clip_a.x + 1.f) * 0.5f) * (static_cast<f32>(WIDTH - 1)),
        ((1.f - clip_a.y) * 0.5f) * (static_cast<f32>(HEIGHT - 1)),
        ws_a.z
    );

    auto b = Vec4<f32>(
        ((clip_b.x + 1.f) * 0.5f) * (static_cast<f32>(WIDTH - 1)),
        ((1.f - clip_b.y) * 0.5f) * (static_cast<f32>(HEIGHT - 1)),
        ws_b.z
    );

    auto c = Vec4<f32>(
        ((clip_c.x + 1.f) * 0.5f) * (static_cast<f32>(WIDTH - 1)),
        ((1.f - clip_c.y) * 0.5f) * (static_cast<f32>(HEIGHT - 1)),
        ws_c.z
    );

    std::cout << "\nScreen space co-ordinates: \n";
    std::cout << a << '\n';
    std::cout << b << '\n';
    std::cout << c << '\n';

    auto edge_ab = ws_b - ws_a;
    auto edge_ac = ws_c - ws_a;

    Vec4<f32> face_normal = edge_ab.cross(edge_ac).normalized();

    f32 brightness = std::max(0.f, face_normal.dot(light_dir));

    draw_triangle(framebuffer, a, b, c, 255, 0, 0, brightness);

    framebuffer.save_ppm(outfile);
    outfile.close();

    return EXIT_SUCCESS;
}
