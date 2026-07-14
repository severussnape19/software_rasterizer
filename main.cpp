#include <chrono>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <print>

import math;
import types;
import parser;
import display;
import rasterize;

auto main(i32 argc, char* argv[]) -> i32 {
    if (argc < 2) {
        std::cerr << "[ERR] File path where bish?\n";
        return EXIT_FAILURE;
    }
    constexpr usize WIDTH = 1920;
    constexpr usize HEIGHT = 1080;

    auto framebuffer = Framebuffer(HEIGHT, WIDTH);

    std::fstream outfile(argv[1], std::ios::out);
    if (!outfile.is_open()) {
        std::cerr << "could not open file!\n";
        return EXIT_FAILURE;
    }

    Vec4<f32> light_dir = Vec4<f32>(0.f, 0.f, 1.f, 0.f).normalized();
    Vec4<f32> view_dir  = Vec4<f32>(0.f, 0.f, -1.f, 0.f);

    auto model_matrix = Mat4<f32>::translation_matrix(-10.f, 0.f, -75.f) * Mat4<f32>::rotation_y(0.f) * Mat4<f32>::scale(0.5f, 0.5f, 0.5f);
    auto view_matrix  = Mat4<f32>::identity_matrix();
    auto projection_matrix = Mat4<f32>::projection_matrix();

    Mat4<f32> mvp = projection_matrix * view_matrix * model_matrix;
    auto mesh = mesh_loader("/home/lakshya/ws/obj_files/common-3d-test-models-master/data/xyzrgb_dragon.obj");

    auto start = std::chrono::high_resolution_clock::now();
    for (auto const& face : mesh.faces) {
        Vec4<f32> ws_a = mesh.vertices[face.a];
        Vec4<f32> ws_b = mesh.vertices[face.b];
        Vec4<f32> ws_c = mesh.vertices[face.c];

        // Flat shading / backward culling
        auto edge_ab = ws_b - ws_a;
        auto edge_ac = ws_c - ws_a;

        Vec4<f32> face_normal = edge_ab.cross(edge_ac).normalized();

        if (face_normal.dot(light_dir) <= 0) {
            // If their dot is -ve, surface normal faces into the world
            // If the dot is +ve, they are in opposite directions so we consider
            continue;
        }

        f32 brightness = std::max(0.f, face_normal.dot(light_dir));

        auto a = to_screen(ws_a, mvp, WIDTH, HEIGHT);
        auto b = to_screen(ws_b, mvp, WIDTH, HEIGHT);
        auto c = to_screen(ws_c, mvp, WIDTH, HEIGHT);

        draw_triangle(framebuffer, a, b, c, 255, 255, 255, brightness);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::println("Elapsed: {}", elapsed);

    std::println("Loaded!!");
    framebuffer.save_ppm(outfile);
    outfile.close();

    return EXIT_SUCCESS;
}
