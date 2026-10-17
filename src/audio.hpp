// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <vector>

#include "mixer.hpp"
#include "synth.hpp"

struct Audio {
    SDL_AudioStream* stream = nullptr;
    Mixer mixer;
    std::array<std::vector<float>, static_cast<size_t>(Sound::Count)> fixed;
    std::array<std::array<std::vector<float>, kMaxClearSteps>, kMaxClearLines> clears;
};

bool audio_init(Audio& audio, int volume_percent);
void audio_shutdown(Audio& audio);
void audio_set_volume(Audio& audio, int volume_percent);
void audio_play(Audio& audio, Sound sound);
void audio_play_clear(Audio& audio, int lines, int streak);
