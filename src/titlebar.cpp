// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "titlebar.hpp"

#include <algorithm>

#include "layout.hpp"

namespace {

constexpr float kButtonW = 44.0f;
constexpr float kResizeBorder = 6.0f;

}

bool titlebar_supported()
{
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)
    return true;
#else
    return false;
#endif
}

int titlebar_hit(float x, float y)
{
    if (y < 0 || y >= kTitleBarH || x < 0 || x >= kScreenW) {
        return kBarNone;
    }
    if (x >= kScreenW - kButtonW) {
        return kBarClose;
    }
    if (x >= kScreenW - 2 * kButtonW) {
        return kBarMinimize;
    }
    return kBarDrag;
}

void draw_titlebar(Gfx& g)
{
    fill_rect(g, {0, 0, kScreenW, kTitleBarH}, palette::kBedEdge);
    TextStyle title{15, 700};
    draw_text_in_line(g, "Grid", title, 14, 0, kTitleBarH, palette::kDim);
    float min_x = kScreenW - 2 * kButtonW;
    fill_rect(g, {min_x + (kButtonW - 12) / 2, kTitleBarH / 2 - 1, 12, 2}, palette::kDim);
    TextStyle close{22, 500};
    draw_text_centered(g, "\xC3\x97", close, {kScreenW - kButtonW, 0, kButtonW, kTitleBarH}, palette::kDim);
}

SDL_HitTestResult SDLCALL titlebar_hit_test(SDL_Window* window, const SDL_Point* point, void*)
{
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
        return SDL_HITTEST_NORMAL;
    }
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(window, &w, &h);
    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);
    float px = static_cast<float>(point->x);
    float py = static_cast<float>(point->y);
    bool left = px < kResizeBorder;
    bool right = px >= fw - kResizeBorder;
    bool top = py < kResizeBorder;
    bool bottom = py >= fh - kResizeBorder;
    if (top && left) {
        return SDL_HITTEST_RESIZE_TOPLEFT;
    }
    if (top && right) {
        return SDL_HITTEST_RESIZE_TOPRIGHT;
    }
    if (bottom && left) {
        return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    }
    if (bottom && right) {
        return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
    }
    if (top) {
        return SDL_HITTEST_RESIZE_TOP;
    }
    if (bottom) {
        return SDL_HITTEST_RESIZE_BOTTOM;
    }
    if (left) {
        return SDL_HITTEST_RESIZE_LEFT;
    }
    if (right) {
        return SDL_HITTEST_RESIZE_RIGHT;
    }
    float full_h = static_cast<float>(kScreenH) + kTitleBarH;
    float scale = std::min(fw / kScreenW, fh / full_h);
    float ox = (fw - kScreenW * scale) / 2;
    float oy = (fh - full_h * scale) / 2;
    int hit = titlebar_hit((px - ox) / scale, (py - oy) / scale);
    return hit == kBarDrag ? SDL_HITTEST_DRAGGABLE : SDL_HITTEST_NORMAL;
}
