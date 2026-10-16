// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "gfx.hpp"

#include <cmath>

#include "layout.hpp"

namespace {

SDL_Texture* bake_vignette(SDL_Renderer* renderer)
{
    SDL_Surface* surf = SDL_CreateSurface(kScreenW, kScreenH, SDL_PIXELFORMAT_RGBA32);
    Uint8* pixels = static_cast<Uint8*>(surf->pixels);
    for (int y = 0; y < kScreenH; y++) {
        for (int x = 0; x < kScreenW; x++) {
            float dx = (static_cast<float>(x) + 0.5f - 270.0f) / 432.0f;
            float dy = (static_cast<float>(y) + 0.5f - 326.4f) / 576.0f;
            float d = std::sqrt(dx * dx + dy * dy);
            float a = d <= 0.36f ? 0.0f : (d >= 1.0f ? 0.94f : 0.94f * (d - 0.36f) / 0.64f);
            Uint8* p = pixels + y * surf->pitch + x * 4;
            p[0] = 16;
            p[1] = 28;
            p[2] = 38;
            p[3] = static_cast<Uint8>(a * 255.0f + 0.5f);
        }
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

}

bool gfx_init(Gfx& g, SDL_Renderer* renderer, const std::string& font_path)
{
    g.renderer = renderer;
    g.font_path = font_path;
    g.vignette = bake_vignette(renderer);
    return g.vignette != nullptr;
}

void gfx_destroy(Gfx& g)
{
    for (auto& [key, s] : g.sprites) {
        SDL_DestroyTexture(s.tex);
    }
    for (auto& [key, t] : g.texts) {
        SDL_DestroyTexture(t.tex);
    }
    for (auto& [key, f] : g.fonts) {
        if (f) {
            TTF_CloseFont(f);
        }
    }
    g.sprites.clear();
    g.texts.clear();
    g.fonts.clear();
    SDL_DestroyTexture(g.vignette);
    g.vignette = nullptr;
}

void gfx_reset(Gfx& g)
{
    for (auto& [key, s] : g.sprites) {
        SDL_DestroyTexture(s.tex);
    }
    for (auto& [key, t] : g.texts) {
        SDL_DestroyTexture(t.tex);
    }
    g.sprites.clear();
    g.texts.clear();
    SDL_DestroyTexture(g.vignette);
    g.vignette = bake_vignette(g.renderer);
}

void gfx_begin_frame(Gfx& g)
{
    g.frame++;
}

Sprite sprite_from_canvas(Gfx& g, const Canvas& canvas)
{
    std::vector<uint8_t> bytes = to_straight_rgba(canvas);
    SDL_Surface* surf = SDL_CreateSurfaceFrom(canvas.w, canvas.h, SDL_PIXELFORMAT_RGBA32, bytes.data(), canvas.w * 4);
    Sprite s;
    s.tex = SDL_CreateTextureFromSurface(g.renderer, surf);
    SDL_DestroySurface(surf);
    SDL_SetTextureScaleMode(s.tex, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(s.tex, SDL_BLENDMODE_BLEND);
    s.ox = -canvas.margin;
    s.oy = -canvas.margin;
    s.w = static_cast<float>(canvas.w) / kBakeScale;
    s.h = static_cast<float>(canvas.h) / kBakeScale;
    return s;
}

void draw_sprite(Gfx& g, const Sprite& s, float x, float y, float alpha)
{
    SDL_SetTextureAlphaMod(s.tex, static_cast<Uint8>(alpha * 255.0f + 0.5f));
    SDL_FRect dst = {x + s.ox, y + s.oy, s.w, s.h};
    SDL_RenderTexture(g.renderer, s.tex, nullptr, &dst);
}

void draw_sprite_scaled(Gfx& g, const Sprite& s, float ax, float ay, float cx, float cy, float scale, float alpha)
{
    SDL_SetTextureAlphaMod(s.tex, static_cast<Uint8>(alpha * 255.0f + 0.5f));
    SDL_FRect dst = {cx + (ax + s.ox - cx) * scale, cy + (ay + s.oy - cy) * scale, s.w * scale, s.h * scale};
    SDL_RenderTexture(g.renderer, s.tex, nullptr, &dst);
}

void fill_rect(Gfx& g, Rect r, Color c)
{
    SDL_SetRenderDrawBlendMode(g.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g.renderer, c.r, c.g, c.b, c.a);
    SDL_FRect f = {r.x, r.y, r.w, r.h};
    SDL_RenderFillRect(g.renderer, &f);
}
