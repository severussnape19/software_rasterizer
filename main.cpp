#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <print>
#include <vector>

import math;
import types;
import parser;

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
    for (usize col{min_x}; col < max_x; ++col) {
        for (usize row{min_y}; row < max_y; ++row) {
            auto p = Vec3<f32>(col, row);

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

                fb.set_pixel(col, row, ir, ig, ib, depth);
            }
        }
    }
}

auto to_screen( Vec4<f32> const& v, Mat4<f32> const& mvp, usize WIDTH, usize HEIGHT ) -> Vec4<f32> {
    Vec4<f32> clip = mvp * v;
    auto const projected_z = clip.z;
    // Perspective divide
    clip /= clip.w;

    // Pixel space conversion
    auto pxl_space_point = Vec4<f32>(
        ((clip.x + 1.f) * 0.5f) * (static_cast<f32>(WIDTH - 1)),
        ((1.f - clip.y) * 0.5f) * (static_cast<f32>(HEIGHT - 1)),
        projected_z
    );

    return pxl_space_point;
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

    Vec3<f32> light_dir = Vec3<f32>(0.f, 0.f, 1.f).normalized();

    auto model_matrix = Mat4<f32>::translation_matrix(-10.f, 0.f, -75.f) * Mat4<f32>::rotation_y(0.f) * Mat4<f32>::scale(0.5f, 0.5f, 0.5f);
    auto view_matrix  = Mat4<f32>::identity_matrix();
    auto projection_matrix = Mat4<f32>::projection_matrix();

    Mat4<f32> mvp = projection_matrix * view_matrix * model_matrix;
    auto mesh = mesh_loader("/home/lakshya/ws/obj_files/common-3d-test-models-master/data/xyzrgb_dragon.obj");

    for (auto const& face : mesh.faces) {
        Vec4<f32> ws_a = mesh.vertices[face.a];
        Vec4<f32> ws_b = mesh.vertices[face.b];
        Vec4<f32> ws_c = mesh.vertices[face.c];

        auto edge_ab = ws_b - ws_a;
        auto edge_ac = ws_c - ws_a;

        Vec4<f32> face_normal = edge_ab.cross(edge_ac).normalized();
        f32 brightness = std::max(0.f, face_normal.dot(light_dir));

        auto a = to_screen(ws_a, mvp, WIDTH, HEIGHT);
        auto b = to_screen(ws_b, mvp, WIDTH, HEIGHT);
        auto c = to_screen(ws_c, mvp, WIDTH, HEIGHT);

        draw_triangle(framebuffer, a, b, c, 255, 255, 255, brightness);
    }

    std::println("Loaded!!");
    framebuffer.save_ppm(outfile);
    outfile.close();

    return EXIT_SUCCESS;
}
