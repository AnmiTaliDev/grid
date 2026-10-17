// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <SDL3/SDL.h>

#include "audio.hpp"
#include "bests.hpp"
#include "game.hpp"
#include "gfx.hpp"
#include "settings.hpp"
#include "views.hpp"

enum class Screen {
    Menu,
    Play,
    Pause,
    Settings,
    Pieces,
    Over,
};

struct App {
    SDL_Window* window = nullptr;
    Screen screen = Screen::Menu;
    Game game;
    Bests bests;
    int previous_best = 0;
    MenuView menu;
    GameView view;
    int panel_pressed = -1;
    bool over_pending = false;
    float over_ms = 0;
    SDL_FingerID finger = 0;
    bool finger_down = false;
    bool quit = false;
    float bar_h = 0;
    Settings settings;
    bool reset_armed = false;
    Audio audio;
    Game undo_game;
    bool has_undo = false;
    int undos_left = 0;
    bool undo_pressed = false;
};

void app_init(App& app, Gfx& g, SDL_Window* window);
void app_event(App& app, Gfx& g, SDL_Event& event);
void app_update(App& app, float dt_ms);
void app_draw(App& app, Gfx& g);
void app_shutdown(App& app);
