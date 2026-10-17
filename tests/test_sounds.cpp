// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "synth.hpp"
#include "wav.hpp"

namespace {

int failures = 0;

void check_file(const std::string& name, const std::vector<float>& expected)
{
    std::string path = std::string(GRID_ASSETS_DIR) + "/sounds/" + name;
    SDL_AudioSpec spec;
    Uint8* data = nullptr;
    Uint32 length = 0;
    if (!SDL_LoadWAV(path.c_str(), &spec, &data, &length)) {
        std::printf("FAIL %s cannot load: %s\n", name.c_str(), SDL_GetError());
        failures++;
        return;
    }
    bool ok = spec.format == SDL_AUDIO_S16 && spec.channels == 1 && spec.freq == kSampleRate;
    std::vector<int16_t> want = quantize_samples(expected);
    ok = ok && length == want.size() * sizeof(int16_t);
    int peak = 0;
    if (ok) {
        const int16_t* got = reinterpret_cast<const int16_t*>(data);
        for (size_t i = 0; i < want.size(); i++) {
            ok = ok && got[i] == want[i];
            peak = std::max(peak, std::abs(static_cast<int>(got[i])));
        }
        ok = ok && peak > 1500;
        ok = ok && std::abs(static_cast<int>(got[want.size() - 1])) < 700;
    }
    SDL_free(data);
    std::printf("%s %s (%zu samples, peak %d)\n", ok ? "PASS" : "FAIL", name.c_str(), want.size(), peak);
    if (!ok) {
        failures++;
    }
}

}

int main()
{
    SDL_Init(0);
    for (int i = 0; i < static_cast<int>(Sound::Count); i++) {
        Sound s = static_cast<Sound>(i);
        check_file(sound_file_name(s), synth_sound(s));
    }
    for (int lines = 1; lines <= kMaxClearLines; lines++) {
        for (int step = 0; step < kMaxClearSteps; step++) {
            check_file(clear_file_name(lines, step), synth_clear(lines, step + 1));
        }
    }
    std::printf("%d sound file failures\n", failures);
    SDL_Quit();
    return failures == 0 ? 0 : 1;
}
