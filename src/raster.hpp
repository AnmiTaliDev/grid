// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <vector>

#include "palette.hpp"

constexpr float kBakeScale = 2.0f;

struct Rect {
    float x;
    float y;
    float w;
    float h;
};

struct Canvas {
    int w = 0;
    int h = 0;
    float margin = 0;
    std::vector<float> px;
};

struct Mask {
    int w = 0;
    int h = 0;
    std::vector<float> a;
};

Canvas make_canvas(float logical_w, float logical_h, float margin);

Mask mask_rr(const Canvas& canvas, Rect r, float radius);
Mask mask_dashed_rr(const Canvas& canvas, Rect r, float radius, float stroke, float dash, float gap);
void mask_cut(Mask& a, const Mask& b);
void mask_blur(Mask& m, float sigma_px);
void paint(Canvas& canvas, const Mask& m, Color c);

void fill_rr(Canvas& canvas, Rect r, float radius, Color c);
void ring_rr(Canvas& canvas, Rect r, float radius, float thickness, Color c);
void inset_band(Canvas& canvas, Rect r, float radius, float depth, Color c);
void shadow_rr(Canvas& canvas, Rect r, float radius, float dy, float blur, float spread, Color c);

std::vector<uint8_t> to_straight_rgba(const Canvas& canvas);
