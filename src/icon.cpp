// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "icon.hpp"

#include <cstring>

#include "raster.hpp"

namespace {

constexpr float kScale = 0.5f;
constexpr float kIconSize = 512.0f * kScale;

float s(float v)
{
    return v * kScale;
}

void paint_glint(Canvas& c, float x, float y)
{
    Color white = with_alpha(rgb(0xFFFFFF), 0.55f);
    fill_rr(c, {s(x + 10.88f), s(y + 9.6f), s(19.2f), s(5.76f)}, s(2.88f), white);
    fill_rr(c, {s(x + 33.28f), s(y + 9.6f), s(5.76f), s(5.76f)}, s(2.88f), white);
}

void paint_tile(Canvas& c, float x, float y)
{
    fill_rr(c, {s(x), s(y + 4), s(64), s(64)}, s(12), palette::kPersimmonEdge);
    fill_rr(c, {s(x), s(y), s(64), s(64)}, s(12), palette::kPersimmonFace);
    paint_glint(c, x, y);
}

void paint_well(Canvas& c, float x, float y)
{
    Rect face = {s(x), s(y), s(64), s(64)};
    fill_rr(c, {s(x), s(y + 1), s(64), s(64)}, s(12), with_alpha(rgb(0xFFFFFF), 0.05f));
    fill_rr(c, face, s(12), palette::kWell);
    inset_band(c, face, s(12), s(2), with_alpha(rgb(0x000000), 0.55f));
}

}

SDL_Surface* make_window_icon()
{
    const char* pattern[5] = {"wttw", "twwt", "twww", "twtt", "wttw"};
    Canvas c = make_canvas(kIconSize, kIconSize, 0);
    fill_rr(c, {0, 0, kIconSize, kIconSize}, s(112), palette::kSlate);
    ring_rr(c, {0, 0, kIconSize, kIconSize}, s(112), s(1), palette::kSlateRing);
    for (int r = 0; r < 5; r++) {
        for (int col = 0; col < 4; col++) {
            float x = 119.0f + 70.0f * static_cast<float>(col);
            float y = 82.0f + 70.0f * static_cast<float>(r);
            if (pattern[r][col] == 't') {
                paint_tile(c, x, y);
            } else {
                paint_well(c, x, y);
            }
        }
    }
    std::vector<uint8_t> bytes = to_straight_rgba(c);
    SDL_Surface* icon = SDL_CreateSurface(c.w, c.h, SDL_PIXELFORMAT_RGBA32);
    std::memcpy(icon->pixels, bytes.data(), bytes.size());
    return icon;
}
