// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "gfx.hpp"
#include "settings.hpp"
#include "views.hpp"
#include "widgets.hpp"

struct OverLayout {
    Rect panel;
    float title_top;
    float score_top;
    Rect chip;
    float line_top;
    Rect again;
    Rect menu;
};

struct PauseLayout {
    Rect panel;
    float title_top;
    Rect resume;
    Rect menu;
};

struct SettingsLayout {
    Rect panel;
    float title_top;
    int count;
    int id[4];
    Rect option[4];
    Rect back;
};

ButtonSpec menu_button(int which);
SettingsLayout settings_layout();
std::string settings_label(const SettingsView& v, int id);
ButtonSpec settings_option_button(const SettingsLayout& l, const SettingsView& v, int index);
ButtonSpec settings_back_button(const SettingsLayout& l);
OverLayout over_layout(Gfx& g, bool chip);
PauseLayout pause_layout();
ButtonSpec over_again_button(const OverLayout& l);
ButtonSpec over_menu_button(const OverLayout& l);
ButtonSpec pause_resume_button(const PauseLayout& l);
ButtonSpec pause_menu_button(const PauseLayout& l);

void draw_header_score(Gfx& g, int score, float alpha);
void draw_backdrop(Gfx& g);
void draw_menu(Gfx& g, const MenuView& v);
void draw_settings(Gfx& g, const SettingsView& v);
void draw_game(Gfx& g, const GameView& v);
void draw_game_over(Gfx& g, const OverView& v);
void draw_pause(Gfx& g, const PauseView& v);
bool warm_step(Gfx& g, int step);
