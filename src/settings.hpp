// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

constexpr int kGridSizes[] = {6, 7, 8};
constexpr int kHandLifts[] = {0, 60, 100};

enum SettingId {
    kSettingGrid,
    kSettingLift,
    kSettingFullscreen,
    kSettingReset,
};

constexpr int kSettingsBack = 4;

struct Settings {
    int grid = 8;
    int lift = 100;
    bool fullscreen = false;
};

bool fullscreen_supported();
int next_grid(int grid);
int next_lift(int lift);
Settings load_settings();
void save_settings(const Settings& settings);
