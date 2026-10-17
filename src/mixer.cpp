// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "mixer.hpp"

#include <algorithm>

void mixer_play(Mixer& mixer, const std::vector<float>& samples, float gain)
{
    if (samples.empty()) {
        return;
    }
    if (mixer.voices.size() >= kMaxVoices) {
        mixer.voices.erase(mixer.voices.begin());
    }
    mixer.voices.push_back({&samples, 0, gain});
}

void mixer_render(Mixer& mixer, float* out, int frames)
{
    std::fill(out, out + frames, 0.0f);
    for (Voice& v : mixer.voices) {
        size_t left = v.samples->size() - v.position;
        size_t count = std::min(left, static_cast<size_t>(frames));
        for (size_t i = 0; i < count; i++) {
            out[i] += (*v.samples)[v.position + i] * v.gain * mixer.volume;
        }
        v.position += count;
    }
    for (int i = 0; i < frames; i++) {
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
    }
    mixer.voices.erase(std::remove_if(mixer.voices.begin(), mixer.voices.end(),
                                      [](const Voice& v) { return v.position >= v.samples->size(); }),
                       mixer.voices.end());
}
