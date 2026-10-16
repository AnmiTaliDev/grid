// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>

#include "board.hpp"
#include "deal.hpp"

constexpr float kBoneHoldMs = 120.0f;
constexpr float kBoneFadeMs = 220.0f;
constexpr float kPopupMs = 700.0f;
constexpr float kReturnMs = 120.0f;

enum MenuButton {
    kMenuPlay,
    kMenuSettings,
    kMenuQuit,
};

struct MenuView {
    int best = 0;
    int focus = -1;
    int pressed = -1;
};

struct GameView {
    Board board;
    uint64_t bone_mask = 0;
    float bone_ms = 0;
    int score = 0;
    int best = 0;
    int streak = 0;
    Tray tray{-1, -1, -1};
    int hand_slot = -1;
    float pointer_x = 0;
    float pointer_y = 0;
    int return_slot = -1;
    float return_t = 0;
    bool popup = false;
    int popup_gain = 0;
    int popup_lines = 0;
    float popup_ms = 0;
    bool pause_pressed = false;
};

struct OverView {
    Board board;
    int score = 0;
    int previous_best = 0;
    bool new_best = false;
    int rescues_left = 0;
    int pressed = -1;
};

struct SettingsView {
    int grid = 8;
    int lift = 100;
    int rescues = 1;
    bool fullscreen = false;
    bool reset_armed = false;
    int pressed = -1;
};

struct PauseView {
    Board board;
    int score = 0;
    int pressed = -1;
};
