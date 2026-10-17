// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>

#include "bests.hpp"
#include "i18n.hpp"
#include "board.hpp"
#include "deal.hpp"
#include "stats.hpp"

constexpr float kBoneHoldMs = 120.0f;
constexpr float kBoneFadeMs = 220.0f;
constexpr float kPopupMs = 700.0f;
constexpr float kReturnMs = 120.0f;

enum MenuButton {
    kMenuPlay,
    kMenuSettings,
    kMenuQuit,
    kMenuContinue,
    kMenuStats,
};

struct MenuView {
    Lang lang = Lang::English;
    int best = 0;
    int best_size = 8;
    bool has_save = false;
    int focus = -1;
    int pressed = -1;
};

struct GameView {
    Lang lang = Lang::English;
    Board board;
    uint64_t bone_mask = 0;
    float bone_ms = 0;
    int score = 0;
    int best = 0;
    int streak = 0;
    Tray tray{-1, -1, -1};
    int hold = -1;
    bool hold_enabled = false;
    int undos_left = 0;
    bool undo_enabled = false;
    bool undo_available = false;
    bool undo_pressed = false;
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

inline int view_piece(const GameView& v, int slot)
{
    if (slot == kTraySize) {
        return v.hold;
    }
    return slot >= 0 && slot < kTraySize ? v.tray[slot] : -1;
}

struct OverView {
    Lang lang = Lang::English;
    Board board;
    int score = 0;
    int previous_best = 0;
    bool new_best = false;
    int rescues_left = 0;
    int pressed = -1;
};

struct SettingsView {
    Lang lang = Lang::English;
    int grid = 8;
    int lift = 100;
    int rescues = 1;
    int undos = 1;
    bool hold = true;
    int volume = 70;
    bool vibration = true;
    int deal = 0;
    int shapes_on = 0;
    int shapes_total = 0;
    bool fullscreen = false;
    bool reset_armed = false;
    int pressed = -1;
};

struct StatsView {
    Lang lang = Lang::English;
    Stats stats;
    Bests bests;
    int pressed = -1;
};

struct TutorialView {
    Lang lang = Lang::English;
    int page = 0;
    float seconds = 0;
    int pressed = -1;
};

struct PiecesView {
    Lang lang = Lang::English;
    uint32_t shapes = 0;
    int pressed = -1;
};

struct PauseView {
    Lang lang = Lang::English;
    Board board;
    int score = 0;
    int pressed = -1;
};
