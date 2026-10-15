// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cmath>
#include <vector>

#include "gfx.hpp"

namespace {

constexpr int kTextPad = 8;
constexpr size_t kTextCacheLimit = 160;

TTF_Font* font_for(Gfx& g, TextStyle style)
{
    int size = static_cast<int>(std::lround(style.size));
    auto key = std::make_pair(size, style.weight);
    auto it = g.fonts.find(key);
    if (it != g.fonts.end()) {
        return it->second;
    }
    int instance = std::clamp(style.weight / 100 - 1, 0, 8);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, TTF_PROP_FONT_CREATE_FILENAME_STRING, g.font_path.c_str());
    SDL_SetFloatProperty(props, TTF_PROP_FONT_CREATE_SIZE_FLOAT, static_cast<float>(size) * kBakeScale);
    SDL_SetNumberProperty(props, TTF_PROP_FONT_CREATE_FACE_NUMBER, static_cast<Sint64>(instance + 1) << 16);
    TTF_Font* font = TTF_OpenFontWithProperties(props);
    SDL_DestroyProperties(props);
    if (font) {
        TTF_SetFontHinting(font, TTF_HINTING_NONE);
        TTF_SetFontKerning(font, true);
    }
    g.fonts[key] = font;
    return font;
}

std::vector<Uint32> decode(const std::string& text)
{
    std::vector<Uint32> out;
    const char* p = text.c_str();
    size_t left = text.size();
    while (left > 0) {
        Uint32 ch = SDL_StepUTF8(&p, &left);
        if (ch == 0) {
            break;
        }
        out.push_back(ch);
    }
    return out;
}

float advance_total(TTF_Font* font, const std::vector<Uint32>& chars, float spacing_px)
{
    float pen = 0;
    Uint32 prev = 0;
    for (Uint32 ch : chars) {
        int kern = 0;
        if (prev) {
            TTF_GetGlyphKerning(font, prev, ch, &kern);
        }
        int adv = 0;
        TTF_GetGlyphMetrics(font, ch, nullptr, nullptr, nullptr, nullptr, &adv);
        pen += static_cast<float>(kern + adv) + spacing_px;
        prev = ch;
    }
    return pen;
}

std::string text_key(const std::string& text, TextStyle style)
{
    return std::to_string(static_cast<int>(std::lround(style.size))) + ":" + std::to_string(style.weight) + ":" +
           std::to_string(style.spacing) + ":" + text;
}

void trim_cache(Gfx& g)
{
    if (g.texts.size() <= kTextCacheLimit) {
        return;
    }
    for (auto it = g.texts.begin(); it != g.texts.end();) {
        if (it->second.used + 1 < g.frame) {
            SDL_DestroyTexture(it->second.tex);
            it = g.texts.erase(it);
        } else {
            ++it;
        }
    }
}

TextEntry& text_entry(Gfx& g, const std::string& text, TextStyle style)
{
    std::string key = text_key(text, style);
    auto it = g.texts.find(key);
    if (it != g.texts.end()) {
        it->second.used = g.frame;
        return it->second;
    }
    trim_cache(g);
    TextEntry entry;
    entry.used = g.frame;
    TTF_Font* font = font_for(g, style);
    std::vector<Uint32> chars = decode(text);
    float spacing_px = style.spacing * kBakeScale;
    float total = advance_total(font, chars, spacing_px);
    entry.width = total / kBakeScale;
    entry.pad = static_cast<float>(kTextPad) / kBakeScale;
    int asc = TTF_GetFontAscent(font);
    int height = TTF_GetFontHeight(font);
    int w = static_cast<int>(std::ceil(total)) + 2 * kTextPad;
    int h = height + 2 * kTextPad;
    SDL_Surface* surf = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    SDL_FillSurfaceRect(surf, nullptr, SDL_MapSurfaceRGBA(surf, 255, 255, 255, 0));
    float pen = 0;
    Uint32 prev = 0;
    for (Uint32 ch : chars) {
        int kern = 0;
        if (prev) {
            TTF_GetGlyphKerning(font, prev, ch, &kern);
        }
        pen += static_cast<float>(kern);
        int minx = 0, maxy = 0, adv = 0;
        TTF_GetGlyphMetrics(font, ch, &minx, nullptr, nullptr, &maxy, &adv);
        TTF_ImageType type;
        SDL_Surface* glyph = TTF_GetGlyphImage(font, ch, &type);
        if (glyph) {
            SDL_Rect dst = {static_cast<int>(std::lround(pen)) + minx + kTextPad, asc - maxy + kTextPad, 0, 0};
            SDL_BlitSurface(glyph, nullptr, surf, &dst);
            SDL_DestroySurface(glyph);
        }
        pen += static_cast<float>(adv) + spacing_px;
        prev = ch;
    }
    entry.tex = SDL_CreateTextureFromSurface(g.renderer, surf);
    SDL_DestroySurface(surf);
    SDL_SetTextureScaleMode(entry.tex, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(entry.tex, SDL_BLENDMODE_BLEND);
    return g.texts.emplace(key, entry).first->second;
}

}

FontMetrics font_metrics(Gfx& g, TextStyle style)
{
    TTF_Font* font = font_for(g, style);
    return {static_cast<float>(TTF_GetFontAscent(font)) / kBakeScale,
            static_cast<float>(-TTF_GetFontDescent(font)) / kBakeScale,
            static_cast<float>(TTF_GetFontLineSkip(font)) / kBakeScale};
}

float normal_line_height(Gfx& g, TextStyle style)
{
    return font_metrics(g, style).normal;
}

float text_width(Gfx& g, const std::string& text, TextStyle style)
{
    TTF_Font* font = font_for(g, style);
    return advance_total(font, decode(text), style.spacing * kBakeScale) / kBakeScale;
}

void draw_text(Gfx& g, const std::string& text, TextStyle style, float x, float baseline, Color c, float alpha)
{
    TextEntry& e = text_entry(g, text, style);
    FontMetrics m = font_metrics(g, style);
    float tw = 0;
    float th = 0;
    SDL_GetTextureSize(e.tex, &tw, &th);
    SDL_SetTextureColorMod(e.tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(e.tex, static_cast<Uint8>(alpha * c.a + 0.5f));
    SDL_FRect dst = {x - e.pad, baseline - m.asc - e.pad, tw / kBakeScale, th / kBakeScale};
    SDL_RenderTexture(g.renderer, e.tex, nullptr, &dst);
}

float baseline_for_line(Gfx& g, TextStyle style, float top, float line_height)
{
    FontMetrics m = font_metrics(g, style);
    return top + (line_height - (m.asc + m.desc)) / 2 + m.asc;
}

void draw_text_in_line(Gfx& g, const std::string& text, TextStyle style, float x, float top, float line_height,
                       Color c, float alpha)
{
    draw_text(g, text, style, x, baseline_for_line(g, style, top, line_height), c, alpha);
}

void draw_text_centered(Gfx& g, const std::string& text, TextStyle style, Rect box, Color c, float alpha)
{
    float n = normal_line_height(g, style);
    float w = text_width(g, text, style);
    draw_text_in_line(g, text, style, box.x + (box.w - w) / 2, box.y + (box.h - n) / 2, n, c, alpha);
}

void draw_text_centered_x(Gfx& g, const std::string& text, TextStyle style, float cx, float top, float line_height,
                          Color c)
{
    float w = text_width(g, text, style);
    draw_text_in_line(g, text, style, cx - w / 2, top, line_height, c);
}
