module;

#include <cstdio>
#include <fstream>
#include <ios>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <iostream>

export module parser;

import math;
import types;

export struct Triangle {
    u32 a, b, c;
};

export struct Mesh {
    std::vector<Vec4<f32>> vertices;
    std::vector<Triangle> faces;
};

export auto mesh_loader(std::string const& path) -> Mesh {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("[ERR] Error opening the file!\n");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer{};
    buffer.resize(size);

    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("[ERR] Could not read data into the buffer!\n");
    }
    Mesh mesh{};
    std::string_view remaining(buffer); // View over the whole buffer
    u32 line = 0;
    while (!remaining.empty()) {
        // Get the end of the line. '\n' is the end of the line
        usize line_end = remaining.find('\n');
        // npos is the sentinal value for string_view. ('\0 is sentinal value for raw strings')
        std::string_view sv = (line_end == std::string_view::npos)
            ? remaining
            : remaining.substr(0, line_end);

        if (sv.starts_with("v ")) {

            sv.remove_prefix(2);
            f32 vertices[3]{0.f, 0.f, 0.f};
            i32 count = 0;

            while (!sv.empty() && count < 3) {
                // Start and end of the token
                usize st = sv.find_first_not_of(' ');
                if (st == std::string_view::npos) {
                    break;
                }
                usize end = sv.find_first_of(' ', st);
                size_t token_len = (end == std::string_view::npos) ? sv.size() - st : end - st;

                // Takes start and length of the token NOT THE END IDX!!
                std::string_view number = sv.subview(st, token_len);
                f32 num{};
                if (std::from_chars(number.data(), number.data() + number.size(), num).ec == std::errc::invalid_argument) {
                    throw std::runtime_error("Anomaly!!");
                }

                vertices[count++] = num;

                if (end == std::string_view::npos) {
                    break;
                }
                sv.remove_prefix(end + 1);
            }
            mesh.vertices.push_back(Vec4<f32>(vertices[0], vertices[1], vertices[2], 1.f));
        } else if (sv.starts_with("f ")) {

            sv.remove_prefix(2);
            std::array<u32, 3> face_points{};
            i32 count = 0;

            while (!sv.empty() && count < 3) {
                usize st = sv.find_first_not_of(' ');
                if (st == std::string_view::npos) {
                    break;
                }
                usize end = sv.find_first_of(' ', st);
                size_t token_len = (end == std::string_view::npos) ? sv.size() - st : end - st;
                std::string_view number = sv.subview(st, token_len);
                u32 num{};
                if (std::from_chars(number.data(), number.data() + number.size(), num).ec
                        == std::errc::invalid_argument) {
                    throw std::runtime_error("Anomaly!!");
                }
                face_points[count++] = num;

                if (end == std::string_view::npos) {
                    break;
                }
                sv.remove_prefix(end + 1);
            }
            Triangle triangle {
                .a = face_points[0] - 1,
                .b = face_points[1] - 1,
                .c = face_points[2] - 1
            };
            mesh.faces.push_back(triangle);
        }
        if (line_end != std::string_view::npos) {
            remaining.remove_prefix(line_end + 1);
        } else {
            break;
        }
    }
    std::println("Object Loaded!!");
    return mesh;
}
