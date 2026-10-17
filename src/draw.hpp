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
    Rect rescue;
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
    int id[12];
    Rect option[12];
    Rect back;
};

constexpr int kMaxShapes = 32;

struct PiecesLayout {
    Rect panel;
    float title_top;
    int count;
    Rect card[kMaxShapes];
    Rect back;
};

ButtonSpec menu_button(int which, Lang lang = Lang::English);
struct StatsLayout {
    Rect panel;
    float title_top;
    float rows_top;
    float row_height;
    Rect back;
};

struct TutorialLayout {
    Rect panel;
    float title_top;
    Rect art;
    float caption_top;
    Rect next;
    Rect skip;
};

TutorialLayout tutorial_layout();
ButtonSpec tutorial_next_button(const TutorialLayout& l, bool last, Lang lang = Lang::English);
ButtonSpec tutorial_skip_button(const TutorialLayout& l, Lang lang = Lang::English);
StatsLayout stats_layout();
ButtonSpec stats_back_button(const StatsLayout& l, Lang lang = Lang::English);
PiecesLayout pieces_layout();
ButtonSpec pieces_back_button(const PiecesLayout& l, Lang lang = Lang::English);
SettingsLayout settings_layout();
std::string settings_label(const SettingsView& v, int id);
ButtonSpec settings_option_button(const SettingsLayout& l, const SettingsView& v, int index);
ButtonSpec settings_back_button(const SettingsLayout& l, Lang lang = Lang::English);
OverLayout over_layout(Gfx& g, bool chip, bool rescue, Lang lang = Lang::English);
PauseLayout pause_layout();
ButtonSpec over_rescue_button(const OverLayout& l, int left, Lang lang = Lang::English);
ButtonSpec over_again_button(const OverLayout& l, Lang lang = Lang::English);
ButtonSpec over_menu_button(const OverLayout& l, Lang lang = Lang::English);
ButtonSpec pause_resume_button(const PauseLayout& l, Lang lang = Lang::English);
ButtonSpec pause_menu_button(const PauseLayout& l, Lang lang = Lang::English);

void draw_header_score(Gfx& g, int score, float alpha);
void draw_backdrop(Gfx& g);
void draw_menu(Gfx& g, const MenuView& v);
void draw_settings(Gfx& g, const SettingsView& v);
void draw_pieces(Gfx& g, const PiecesView& v);
void draw_stats(Gfx& g, const StatsView& v);
void draw_tutorial(Gfx& g, const TutorialView& v);
void draw_game(Gfx& g, const GameView& v);
void draw_game_over(Gfx& g, const OverView& v);
void draw_pause(Gfx& g, const PauseView& v);
bool warm_step(Gfx& g, int step);
