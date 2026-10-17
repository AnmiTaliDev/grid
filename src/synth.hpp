// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <string>
#include <vector>

constexpr int kSampleRate = 44100;
constexpr int kMaxClearLines = 4;
constexpr int kMaxClearSteps = 4;

enum class Sound {
    Pick,
    Place,
    Invalid,
    Hold,
    Undo,
    GameOver,
    Rescue,
    Click,
    Count,
};

const char* sound_file_name(Sound sound);
std::string clear_file_name(int lines, int streak_step);
std::vector<float> synth_sound(Sound sound);
std::vector<float> synth_clear(int lines, int streak);
