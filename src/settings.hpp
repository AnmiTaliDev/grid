// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>

constexpr int kGridSizes[] = {6, 7, 8};
constexpr int kHandLifts[] = {0, 60, 100};
constexpr int kRescueCounts[] = {0, 1, 2, 3};
constexpr int kUndoCounts[] = {0, 1, 2, 3};
constexpr int kVolumes[] = {0, 35, 70, 100};

enum SettingId {
    kSettingGrid,
    kSettingLift,
    kSettingFullscreen,
    kSettingRescues,
    kSettingUndo,
    kSettingHold,
    kSettingSound,
    kSettingVibration,
    kSettingDeal,
    kSettingPieces,
    kSettingReset,
};

constexpr int kSettingsBack = 99;

struct Settings {
    int grid = 8;
    int lift = 100;
    int rescues = 1;
    int undos = 1;
    bool hold = true;
    int volume = 70;
    bool vibration = true;
    bool tutorial_seen = false;
    int deal = 0;
    uint32_t shapes = 0;
    bool fullscreen = false;
};

bool fullscreen_supported();
int next_grid(int grid);
int next_lift(int lift);
int next_rescues(int rescues);
int next_undos(int undos);
int next_volume(int volume);
int next_deal(int deal);
Settings load_settings();
void save_settings(const Settings& settings);
