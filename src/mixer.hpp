// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstddef>
#include <vector>

constexpr size_t kMaxVoices = 12;

struct Voice {
    const std::vector<float>* samples;
    size_t position;
    float gain;
};

struct Mixer {
    std::vector<Voice> voices;
    float volume = 0.7f;
};

void mixer_play(Mixer& mixer, const std::vector<float>& samples, float gain);
void mixer_render(Mixer& mixer, float* out, int frames);
