// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "app.hpp"
#include "draw.hpp"
#include "icon.hpp"
#include "layout.hpp"
#include "sprites.hpp"
#include "titlebar.hpp"

namespace {

std::string font_path()
{
#ifdef SDL_PLATFORM_ANDROID
    return "Geologica.ttf";
#else
    const char* base = SDL_GetBasePath();
    return std::string(base ? base : "") + "assets/Geologica.ttf";
#endif
}

}

int main(int argc, char** argv)
{
    int max_frames = -1;
    if (argc == 3 && std::strcmp(argv[1], "--frames") == 0) {
        max_frames = std::atoi(argv[2]);
    }
    SDL_SetAppMetadata("Grid", "0.1.0", "dev.anmitali.grid");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) {
        SDL_Log("Init failed: %s", SDL_GetError());
        return 1;
    }
    float bar = titlebar_supported() ? kTitleBarH : 0.0f;
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | (titlebar_supported() ? SDL_WINDOW_BORDERLESS : 0);
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer("Grid", kScreenW, kScreenH + static_cast<int>(bar), flags, &window, &renderer)) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Surface* icon = make_window_icon();
    SDL_SetWindowIcon(window, icon);
    SDL_DestroySurface(icon);
    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderDrawColor(renderer, 16, 28, 38, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    Gfx gfx;
    if (!gfx_init(gfx, renderer, font_path())) {
        SDL_Log("Graphics init failed: %s", SDL_GetError());
        return 1;
    }
    App app;
    app_init(app, gfx, window);
    Uint64 last = SDL_GetTicksNS();
    int frames = 0;
    int warm = 0;
    bool warming = true;
    while (!app.quit && (max_frames < 0 || frames < max_frames)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            app_event(app, gfx, event);
        }
        Uint64 now = SDL_GetTicksNS();
        while (warming && SDL_GetTicksNS() - now < 6000000) {
            warming = warm_step(gfx, warm++);
        }
        now = SDL_GetTicksNS();
        app_update(app, static_cast<float>(now - last) / 1.0e6f);
        last = now;
        gfx_begin_frame(gfx);
        SDL_SetRenderDrawColor(renderer, 16, 28, 38, 255);
        SDL_RenderClear(renderer);
        app_draw(app, gfx);
        SDL_RenderPresent(renderer);
        frames++;
    }
    app_shutdown(app);
    gfx_destroy(gfx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
