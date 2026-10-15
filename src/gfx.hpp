// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdint>
#include <map>
#include <string>
#include <utility>

#include "palette.hpp"
#include "raster.hpp"

struct Sprite {
    SDL_Texture* tex = nullptr;
    float ox = 0;
    float oy = 0;
    float w = 0;
    float h = 0;
};

struct SpriteKey {
    int kind = 0;
    float a = 0;
    float b = 0;
    float c = 0;
    float d = 0;
    float e = 0;
    uint32_t color1 = 0;
    uint32_t color2 = 0;
    int flags = 0;
    auto operator<=>(const SpriteKey&) const = default;
};

struct TextStyle {
    float size;
    int weight;
    float spacing = 0;
};

struct TextEntry {
    SDL_Texture* tex = nullptr;
    float pad = 0;
    float width = 0;
    uint64_t used = 0;
};

struct Gfx {
    SDL_Renderer* renderer = nullptr;
    std::string font_path;
    std::map<std::pair<int, int>, TTF_Font*> fonts;
    std::map<SpriteKey, Sprite> sprites;
    std::map<std::string, TextEntry> texts;
    SDL_Texture* vignette = nullptr;
    uint64_t frame = 0;
};

bool gfx_init(Gfx& g, SDL_Renderer* renderer, const std::string& font_path);
void gfx_destroy(Gfx& g);
void gfx_begin_frame(Gfx& g);

Sprite sprite_from_canvas(Gfx& g, const Canvas& canvas);
void draw_sprite(Gfx& g, const Sprite& s, float x, float y, float alpha = 1.0f);
void draw_sprite_scaled(Gfx& g, const Sprite& s, float ax, float ay, float cx, float cy, float scale, float alpha);
void fill_rect(Gfx& g, Rect r, Color c);

struct FontMetrics {
    float asc;
    float desc;
    float normal;
};

FontMetrics font_metrics(Gfx& g, TextStyle style);
float text_width(Gfx& g, const std::string& text, TextStyle style);
void draw_text(Gfx& g, const std::string& text, TextStyle style, float x, float baseline, Color c,
               float alpha = 1.0f);
float baseline_for_line(Gfx& g, TextStyle style, float top, float line_height);
float normal_line_height(Gfx& g, TextStyle style);
void draw_text_in_line(Gfx& g, const std::string& text, TextStyle style, float x, float top, float line_height,
                       Color c, float alpha = 1.0f);
void draw_text_centered(Gfx& g, const std::string& text, TextStyle style, Rect box, Color c, float alpha = 1.0f);
void draw_text_centered_x(Gfx& g, const std::string& text, TextStyle style, float cx, float top,
                          float line_height, Color c);
