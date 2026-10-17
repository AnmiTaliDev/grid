// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cmath>

#include "draw.hpp"
#include "haptics.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;

}

static_assert(kSettingsBack > 12, "Back index must not collide with option indices");

SettingsLayout settings_layout()
{
    SettingsLayout l;
    l.count = 0;
    l.id[l.count++] = kSettingGrid;
    l.id[l.count++] = kSettingLift;
    if (fullscreen_supported()) {
        l.id[l.count++] = kSettingFullscreen;
    }
    l.id[l.count++] = kSettingDeal;
    l.id[l.count++] = kSettingPieces;
    l.id[l.count++] = kSettingRescues;
    l.id[l.count++] = kSettingUndo;
    l.id[l.count++] = kSettingHold;
    l.id[l.count++] = kSettingSound;
    l.id[l.count++] = kSettingLanguage;
    if (haptics_supported()) {
        l.id[l.count++] = kSettingVibration;
    }
    l.id[l.count++] = kSettingReset;
    float n = static_cast<float>(l.count);
    float fixed = 36 + kTitleLine + 24 + 28 + 64 + 36;
    float gap = 10;
    float row = std::min(56.0f, std::floor((900.0f - fixed - (n - 1) * gap) / n));
    float h = fixed + n * row + (n - 1) * gap;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    float y = l.title_top + kTitleLine + 24;
    for (int i = 0; i < l.count; i++) {
        l.option[i] = {kContentX, y, kContentW, row};
        y += row + gap;
    }
    l.back = {kContentX, y - gap + 28, kContentW, 64};
    return l;
}

namespace {

std::string with_value(Lang lang, Str name, const std::string& value)
{
    return std::string(tr(lang, name)) + ": " + value;
}

std::string on_off(Lang lang, bool on)
{
    return tr(lang, on ? Str::On : Str::Off);
}

std::string count_or_off(Lang lang, int count)
{
    return count == 0 ? std::string(tr(lang, Str::Off)) : std::to_string(count);
}

}

std::string settings_label(const SettingsView& v, int id)
{
    Lang lang = v.lang;
    if (id == kSettingGrid) {
        return with_value(lang, Str::SetGrid, std::to_string(v.grid) + "x" + std::to_string(v.grid));
    }
    if (id == kSettingLift) {
        return with_value(lang, Str::SetLift, v.lift == 0 ? std::string(tr(lang, Str::None)) : std::to_string(v.lift) + " px");
    }
    if (id == kSettingPieces) {
        return with_value(lang, Str::SetPieces, std::to_string(v.shapes_on) + "/" + std::to_string(v.shapes_total));
    }
    if (id == kSettingDeal) {
        const Str names[3] = {Str::DealRandom, Str::DealFitting, Str::DealDynamic};
        return with_value(lang, Str::SetDeal, tr(lang, names[v.deal]));
    }
    if (id == kSettingVibration) {
        return with_value(lang, Str::SetVibration, on_off(lang, v.vibration));
    }
    if (id == kSettingSound) {
        return with_value(lang, Str::SetSound, v.volume == 0 ? on_off(lang, false) : std::to_string(v.volume) + "%");
    }
    if (id == kSettingUndo) {
        return with_value(lang, Str::SetUndo, count_or_off(lang, v.undos));
    }
    if (id == kSettingHold) {
        return with_value(lang, Str::SetHold, on_off(lang, v.hold));
    }
    if (id == kSettingRescues) {
        return with_value(lang, Str::SetRescues, count_or_off(lang, v.rescues));
    }
    if (id == kSettingLanguage) {
        return with_value(lang, Str::SetLanguage, language_name(lang));
    }
    if (id == kSettingFullscreen) {
        return with_value(lang, Str::SetFullscreen, on_off(lang, v.fullscreen));
    }
    return tr(lang, v.reset_armed ? Str::ConfirmReset : Str::ResetBests);
}

ButtonSpec settings_option_button(const SettingsLayout& l, const SettingsView& v, int index)
{
    return {l.option[index], 16, 4, palette::kPanelSecondary, palette::kBoneEdge, settings_label(v, l.id[index]),
            {18, 600}, palette::kInk};
}

ButtonSpec settings_back_button(const SettingsLayout& l, Lang lang)
{
    return {l.back, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, tr(lang, Str::Back), {24, 800}, palette::kBone};
}

void draw_settings(Gfx& g, const SettingsView& v)
{
    draw_backdrop(g);
    SettingsLayout l = settings_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, tr(v.lang, Str::Settings), {28, 700}, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    for (int i = 0; i < l.count; i++) {
        draw_button(g, settings_option_button(l, v, i), v.pressed == i, false);
    }
    draw_button(g, settings_back_button(l, v.lang), v.pressed == kSettingsBack, false);
}
