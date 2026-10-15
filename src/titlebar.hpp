// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <SDL3/SDL.h>

#include "gfx.hpp"

constexpr float kTitleBarH = 36.0f;

enum TitleBarHit {
    kBarNone,
    kBarDrag,
    kBarMinimize,
    kBarClose,
};

bool titlebar_supported();
int titlebar_hit(float x, float y);
void draw_titlebar(Gfx& g);
SDL_HitTestResult SDLCALL titlebar_hit_test(SDL_Window* window, const SDL_Point* point, void* data);
