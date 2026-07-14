module;

#include <algorithm>
#include <cmath>
export module rasterize;

import types;
import math;
import display;

export auto edge_function(Vec4<f32> const& a, Vec4<f32> const& b, Vec4<f32> const& p) -> f32 {
    auto ba = Vec2<f32>(b.x - a.x, b.y - a.y);
    auto pa = Vec2<f32>(p.x - a.x, p.y - a.y);
    return pa.perp_dot(ba);
}

export auto draw_triangle(
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

export auto to_screen( Vec4<f32> const& v, Mat4<f32> const& mvp, usize WIDTH, usize HEIGHT ) -> Vec4<f32> {
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
