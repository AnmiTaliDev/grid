// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "raster.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979f;

struct PxRect {
    float cx;
    float cy;
    float hx;
    float hy;
    float r;
};

PxRect to_px(const Canvas& canvas, Rect r, float radius)
{
    float x = (r.x + canvas.margin) * kBakeScale;
    float y = (r.y + canvas.margin) * kBakeScale;
    float w = r.w * kBakeScale;
    float h = r.h * kBakeScale;
    float rad = std::min(radius * kBakeScale, std::min(w, h) / 2);
    return {x + w / 2, y + h / 2, w / 2, h / 2, std::max(rad, 0.0f)};
}

float signed_distance(const PxRect& p, float px, float py)
{
    float qx = std::fabs(px - p.cx) - (p.hx - p.r);
    float qy = std::fabs(py - p.cy) - (p.hy - p.r);
    float ox = std::max(qx, 0.0f);
    float oy = std::max(qy, 0.0f);
    return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.0f) - p.r;
}

float coverage(float distance)
{
    return std::clamp(0.5f - distance, 0.0f, 1.0f);
}

void bounds(const Canvas& canvas, const PxRect& p, float pad, int& x0, int& y0, int& x1, int& y1)
{
    x0 = std::max(0, static_cast<int>(std::floor(p.cx - p.hx - pad)));
    y0 = std::max(0, static_cast<int>(std::floor(p.cy - p.hy - pad)));
    x1 = std::min(canvas.w, static_cast<int>(std::ceil(p.cx + p.hx + pad)));
    y1 = std::min(canvas.h, static_cast<int>(std::ceil(p.cy + p.hy + pad)));
}

Mask empty_mask(const Canvas& canvas)
{
    Mask m;
    m.w = canvas.w;
    m.h = canvas.h;
    m.a.assign(static_cast<size_t>(m.w) * m.h, 0.0f);
    return m;
}

float path_position(float qx, float qy, float a, float b, float pr)
{
    float t;
    if (qx > a && qy > b) {
        t = b + pr * std::atan2(qy - b, qx - a);
    } else if (qx - a > qy - b) {
        t = std::min(qy, b);
    } else {
        t = b + pr * kPi / 2 + (a - std::min(qx, a));
    }
    return t;
}

float dash_coverage(float m, float period)
{
    float half = period / 2;
    if (m <= half) {
        return std::clamp(0.5f + std::min(m, half - m), 0.0f, 1.0f);
    }
    return std::clamp(0.5f - std::min(m - half, period - m), 0.0f, 1.0f);
}

void box_blur_line(const float* src, float* dst, int n, int stride, int radius)
{
    float inv = 1.0f / static_cast<float>(2 * radius + 1);
    float sum = 0;
    for (int i = 0; i <= radius && i < n; i++) {
        sum += src[i * stride];
    }
    for (int i = 0; i < n; i++) {
        dst[i * stride] = sum * inv;
        int add = i + radius + 1;
        int sub = i - radius;
        if (add < n) {
            sum += src[add * stride];
        }
        if (sub >= 0) {
            sum -= src[sub * stride];
        }
    }
}

void box_blur(Mask& m, int radius)
{
    std::vector<float> tmp(m.a.size());
    for (int y = 0; y < m.h; y++) {
        box_blur_line(&m.a[static_cast<size_t>(y) * m.w], &tmp[static_cast<size_t>(y) * m.w], m.w, 1, radius);
    }
    for (int x = 0; x < m.w; x++) {
        box_blur_line(&tmp[x], &m.a[x], m.h, m.w, radius);
    }
}

}

Canvas make_canvas(float logical_w, float logical_h, float margin)
{
    Canvas c;
    c.margin = margin;
    c.w = static_cast<int>(std::ceil((logical_w + 2 * margin) * kBakeScale));
    c.h = static_cast<int>(std::ceil((logical_h + 2 * margin) * kBakeScale));
    c.w += c.w % 2;
    c.h += c.h % 2;
    c.px.assign(static_cast<size_t>(c.w) * c.h * 4, 0.0f);
    return c;
}

Mask mask_rr(const Canvas& canvas, Rect r, float radius)
{
    Mask m = empty_mask(canvas);
    PxRect p = to_px(canvas, r, radius);
    int x0, y0, x1, y1;
    bounds(canvas, p, 2.0f, x0, y0, x1, y1);
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            float d = signed_distance(p, static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            m.a[static_cast<size_t>(y) * m.w + x] = coverage(d);
        }
    }
    return m;
}

Mask mask_dashed_rr(const Canvas& canvas, Rect r, float radius, float stroke, float dash, float gap)
{
    Mask m = empty_mask(canvas);
    float inset = stroke / 2;
    PxRect p = to_px(canvas, {r.x + inset, r.y + inset, r.w - stroke, r.h - stroke}, radius - inset);
    float a = p.hx - p.r;
    float b = p.hy - p.r;
    float quarter = a + b + p.r * kPi / 2;
    float perimeter = 4 * quarter;
    float count = std::max(1.0f, std::round(perimeter / ((dash + gap) * kBakeScale)));
    float period = perimeter / count;
    float half_stroke = stroke * kBakeScale / 2;
    int x0, y0, x1, y1;
    bounds(canvas, p, half_stroke + 2.0f, x0, y0, x1, y1);
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            float px = static_cast<float>(x) + 0.5f;
            float py = static_cast<float>(y) + 0.5f;
            float d = signed_distance(p, px, py);
            float stroke_cov = std::clamp(half_stroke + 0.5f - std::fabs(d), 0.0f, 1.0f);
            if (stroke_cov <= 0) {
                continue;
            }
            float dx = px - p.cx;
            float dy = py - p.cy;
            float t = path_position(std::fabs(dx), std::fabs(dy), a, b, p.r);
            float s;
            if (dx >= 0 && dy >= 0) {
                s = t;
            } else if (dx < 0 && dy >= 0) {
                s = 2 * quarter - t;
            } else if (dx < 0) {
                s = 2 * quarter + t;
            } else {
                s = 4 * quarter - t;
            }
            m.a[static_cast<size_t>(y) * m.w + x] = stroke_cov * dash_coverage(std::fmod(s, period), period);
        }
    }
    return m;
}

void mask_cut(Mask& a, const Mask& b)
{
    for (size_t i = 0; i < a.a.size(); i++) {
        a.a[i] *= 1.0f - b.a[i];
    }
}

void mask_blur(Mask& m, float sigma_px)
{
    const int passes = 3;
    float ideal = std::sqrt(12.0f * sigma_px * sigma_px / passes + 1.0f);
    int lower = static_cast<int>(std::floor(ideal));
    if (lower % 2 == 0) {
        lower--;
    }
    int upper = lower + 2;
    float mid = (12.0f * sigma_px * sigma_px - passes * lower * lower - 4.0f * passes * lower - 3.0f * passes) /
                (-4.0f * lower - 4.0f);
    int small_passes = static_cast<int>(std::lround(mid));
    for (int i = 0; i < passes; i++) {
        int size = i < small_passes ? lower : upper;
        box_blur(m, (size - 1) / 2);
    }
}

void paint(Canvas& canvas, const Mask& m, Color c)
{
    float cr = c.r / 255.0f;
    float cg = c.g / 255.0f;
    float cb = c.b / 255.0f;
    float ca = c.a / 255.0f;
    size_t count = m.a.size();
    for (size_t i = 0; i < count; i++) {
        float a = m.a[i] * ca;
        if (a <= 0) {
            continue;
        }
        float* d = &canvas.px[i * 4];
        float inv = 1.0f - a;
        d[0] = cr * a + d[0] * inv;
        d[1] = cg * a + d[1] * inv;
        d[2] = cb * a + d[2] * inv;
        d[3] = a + d[3] * inv;
    }
}

void fill_rr(Canvas& canvas, Rect r, float radius, Color c)
{
    paint(canvas, mask_rr(canvas, r, radius), c);
}

void ring_rr(Canvas& canvas, Rect r, float radius, float thickness, Color c)
{
    Mask outer = mask_rr(canvas, r, radius);
    Mask inner = mask_rr(canvas, {r.x + thickness, r.y + thickness, r.w - 2 * thickness, r.h - 2 * thickness},
                         radius - thickness);
    mask_cut(outer, inner);
    paint(canvas, outer, c);
}

void inset_band(Canvas& canvas, Rect r, float radius, float depth, Color c)
{
    Mask outer = mask_rr(canvas, r, radius);
    Mask shifted = mask_rr(canvas, {r.x, r.y + depth, r.w, r.h}, radius);
    mask_cut(outer, shifted);
    paint(canvas, outer, c);
}

void shadow_rr(Canvas& canvas, Rect r, float radius, float dy, float blur, float spread, Color c)
{
    Mask m = mask_rr(canvas, {r.x - spread, r.y - spread + dy, r.w + 2 * spread, r.h + 2 * spread}, radius + spread);
    mask_blur(m, blur / 2 * kBakeScale);
    paint(canvas, m, c);
}

std::vector<uint8_t> to_straight_rgba(const Canvas& canvas)
{
    size_t count = static_cast<size_t>(canvas.w) * canvas.h;
    std::vector<float> rgb(count * 3, 0.0f);
    std::vector<uint8_t> valid(count, 0);
    std::vector<uint8_t> out(count * 4, 0);
    for (size_t i = 0; i < count; i++) {
        const float* s = &canvas.px[i * 4];
        float a = s[3];
        out[i * 4 + 3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f + 0.5f);
        if (a > 1e-6f) {
            for (int k = 0; k < 3; k++) {
                rgb[i * 3 + k] = std::clamp(s[k] / a, 0.0f, 1.0f);
            }
            valid[i] = 1;
        }
    }
    for (int pass = 0; pass < 2; pass++) {
        std::vector<uint8_t> next = valid;
        for (int y = 0; y < canvas.h; y++) {
            for (int x = 0; x < canvas.w; x++) {
                size_t i = static_cast<size_t>(y) * canvas.w + x;
                if (valid[i]) {
                    continue;
                }
                const int dx[4] = {-1, 1, 0, 0};
                const int dy[4] = {0, 0, -1, 1};
                for (int n = 0; n < 4; n++) {
                    int nx = x + dx[n];
                    int ny = y + dy[n];
                    if (nx < 0 || ny < 0 || nx >= canvas.w || ny >= canvas.h) {
                        continue;
                    }
                    size_t j = static_cast<size_t>(ny) * canvas.w + nx;
                    if (valid[j]) {
                        for (int k = 0; k < 3; k++) {
                            rgb[i * 3 + k] = rgb[j * 3 + k];
                        }
                        next[i] = 1;
                        break;
                    }
                }
            }
        }
        valid = next;
    }
    for (size_t i = 0; i < count; i++) {
        for (int k = 0; k < 3; k++) {
            out[i * 4 + k] = static_cast<uint8_t>(rgb[i * 3 + k] * 255.0f + 0.5f);
        }
    }
    return out;
}
