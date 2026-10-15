// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "draw.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;

}

SettingsLayout settings_layout()
{
    SettingsLayout l;
    float h = 36 + kTitleLine + 24 + 4 * 56 + 3 * 14 + 28 + 64 + 36;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    float y = l.title_top + kTitleLine + 24;
    for (int i = 0; i < 4; i++) {
        l.option[i] = {kContentX, y, kContentW, 56};
        y += 56 + 14;
    }
    l.back = {kContentX, y - 14 + 28, kContentW, 64};
    return l;
}

std::string settings_label(const SettingsView& v, int index)
{
    if (index == 0) {
        return "Grid size: " + std::to_string(v.grid) + "x" + std::to_string(v.grid);
    }
    if (index == 1) {
        return v.lift == 0 ? "Piece lift: none" : "Piece lift: " + std::to_string(v.lift) + " px";
    }
    if (index == 2) {
        return std::string("Fullscreen: ") + (v.fullscreen ? "on" : "off");
    }
    return v.reset_armed ? "Confirm reset" : "Reset best";
}

ButtonSpec settings_option_button(const SettingsLayout& l, const SettingsView& v, int index)
{
    return {l.option[index], 16, 4, palette::kPanelSecondary, palette::kBoneEdge, settings_label(v, index),
            {18, 600}, palette::kInk};
}

ButtonSpec settings_back_button(const SettingsLayout& l)
{
    return {l.back, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, "Back", {24, 800}, palette::kBone};
}

void draw_settings(Gfx& g, const SettingsView& v)
{
    draw_backdrop(g);
    SettingsLayout l = settings_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, "Settings", {28, 700}, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    for (int i = 0; i < 4; i++) {
        draw_button(g, settings_option_button(l, v, i), v.pressed == i, false);
    }
    draw_button(g, settings_back_button(l), v.pressed == 4, false);
}
