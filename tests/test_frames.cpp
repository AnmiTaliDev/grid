// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "draw.hpp"
#include "frames.hpp"
#include "layout.hpp"
#include "sprites.hpp"

namespace {

struct Checkpoint {
    int x;
    int y;
    int r;
    int g;
    int b;
};

int failures = 0;

SDL_Surface* render_frame(Gfx& gfx, void (*draw)(Gfx&))
{
    SDL_SetRenderDrawColor(gfx.renderer, 0, 0, 0, 255);
    SDL_RenderClear(gfx.renderer);
    gfx_begin_frame(gfx);
    draw(gfx);
    return SDL_RenderReadPixels(gfx.renderer, nullptr);
}

void check(SDL_Surface* frame, const char* name, const Checkpoint* points, int count, int tolerance)
{
    for (int i = 0; i < count; i++) {
        const Checkpoint& p = points[i];
        Uint8 r = 0, g = 0, b = 0, a = 0;
        SDL_ReadSurfacePixel(frame, p.x, p.y, &r, &g, &b, &a);
        bool ok = std::abs(r - p.r) <= tolerance && std::abs(g - p.g) <= tolerance && std::abs(b - p.b) <= tolerance;
        std::printf("%s %s (%d,%d) actual %d,%d,%d expected %d,%d,%d\n", ok ? "PASS" : "FAIL", name, p.x, p.y, r, g, b,
                    p.r, p.g, p.b);
        if (!ok) {
            failures++;
        }
    }
}

void draw_menu_frame(Gfx& g)
{
    draw_menu(g, reference_menu());
}

void draw_game_frame(Gfx& g)
{
    draw_game(g, reference_game());
}

void draw_clear_frame(Gfx& g)
{
    draw_game(g, reference_clear());
}

void draw_over_frame(Gfx& g)
{
    draw_game_over(g, reference_over());
}

constexpr Checkpoint kMenu[] = {
    {5, 5, 16, 28, 38},     {30, 14, 14, 26, 36},   {62, 238, 0xEE, 0x6A, 0x4C}, {190, 238, 0xF3, 0xB9, 0x3F},
    {350, 238, 0x6F, 0xD1, 0xAA}, {414, 238, 0x4C, 0x7C, 0xF0}, {300, 550, 0xEF, 0xE9, 0xDC},
    {100, 650, 0x1C, 0x30, 0x40}, {400, 650, 0x1C, 0x30, 0x40},
};

constexpr Checkpoint kGame[] = {
    {5, 5, 0x10, 0x1C, 0x26},     {52, 36, 0x17, 0x27, 0x33},  {48, 52, 0xEF, 0xE9, 0xDC},
    {474, 110, 0xF3, 0xB9, 0x3F}, {34, 300, 0x17, 0x27, 0x33}, {67, 255, 0x0C, 0x16, 0x20},
    {67, 487, 0xEE, 0x6A, 0x4C},  {67, 515, 0xA8, 0x41, 0x2A},     {249, 561, 0x6F, 0xD1, 0xAA}, {85, 788, 0xEE, 0x6A, 0x4C}, {404, 805, 0xC0, 0x7A, 0xE0},
    {50, 885, 0x12, 0x1F, 0x29},  {270, 820, 0x10, 0x1C, 0x26},
};

constexpr Checkpoint kClear[] = {
    {67, 255, 0x0C, 0x16, 0x20}, {473, 255, 0xEF, 0xE9, 0xDC}, {67, 661, 0xEF, 0xE9, 0xDC},
    {183, 487, 0x6F, 0xD1, 0xAA}, {241, 545, 0x4C, 0x7C, 0xF0},
};

constexpr Checkpoint kOver[] = {
    {52, 36, 18, 32, 43}, {34, 230, 19, 22, 25}, {80, 240, 101, 74, 67}, {200, 262, 0xEF, 0xE9, 0xDC},
};

void run(Gfx& gfx, const char* name, void (*draw)(Gfx&), const Checkpoint* points, int count,
         int tolerance, const char* file)
{
    SDL_Surface* frame = render_frame(gfx, draw);
    if (!frame) {
        std::printf("FAIL %s cannot read pixels: %s\n", name, SDL_GetError());
        failures++;
        return;
    }
    check(frame, name, points, count, tolerance);
    SDL_SaveBMP(frame, file);
    SDL_DestroySurface(frame);
}

}

int main()
{
    SDL_setenv_unsafe("SDL_VIDEODRIVER", "dummy", 0);
    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) {
        std::printf("init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface* target = SDL_CreateSurface(kScreenW, kScreenH, SDL_PIXELFORMAT_ARGB8888);
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(target);
    if (!renderer) {
        std::printf("renderer failed: %s\n", SDL_GetError());
        return 1;
    }
    const char* base = SDL_GetBasePath();
    Gfx gfx;
    if (!gfx_init(gfx, renderer, std::string(base ? base : "") + "assets/Geologica.ttf")) {
        std::printf("gfx failed: %s\n", SDL_GetError());
        return 1;
    }
    run(gfx, "menu", draw_menu_frame, kMenu, sizeof(kMenu) / sizeof(kMenu[0]), 3, "frame_menu.bmp");
    run(gfx, "game", draw_game_frame, kGame, sizeof(kGame) / sizeof(kGame[0]), 3, "frame_game.bmp");
    run(gfx, "clear", draw_clear_frame, kClear, sizeof(kClear) / sizeof(kClear[0]), 3, "frame_clear.bmp");
    run(gfx, "over", draw_over_frame, kOver, sizeof(kOver) / sizeof(kOver[0]), 4, "frame_over.bmp");
    gfx_destroy(gfx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(target);
    TTF_Quit();
    SDL_Quit();
    std::printf("%d checkpoint failures\n", failures);
    return failures == 0 ? 0 : 1;
}
