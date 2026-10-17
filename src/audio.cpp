// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "audio.hpp"

#include <algorithm>
#include <string>

#include "assets.hpp"

namespace {

void SDLCALL feed(void* data, SDL_AudioStream* stream, int additional, int)
{
    Audio& audio = *static_cast<Audio*>(data);
    int frames = additional / static_cast<int>(sizeof(float));
    if (frames <= 0) {
        return;
    }
    std::vector<float> block(static_cast<size_t>(frames));
    mixer_render(audio.mixer, block.data(), frames);
    SDL_PutAudioStreamData(stream, block.data(), frames * static_cast<int>(sizeof(float)));
}

}

namespace {

bool load_wav(const std::string& path, std::vector<float>& out)
{
    SDL_AudioSpec spec;
    Uint8* data = nullptr;
    Uint32 length = 0;
    if (!SDL_LoadWAV(path.c_str(), &spec, &data, &length)) {
        SDL_Log("Cannot load %s: %s", path.c_str(), SDL_GetError());
        return false;
    }
    bool ok = spec.format == SDL_AUDIO_S16 && spec.channels == 1 && spec.freq == kSampleRate;
    if (!ok) {
        SDL_Log("Unexpected format in %s", path.c_str());
    } else {
        const Sint16* pcm = reinterpret_cast<const Sint16*>(data);
        size_t count = length / sizeof(Sint16);
        out.resize(count);
        for (size_t i = 0; i < count; i++) {
            out[i] = static_cast<float>(pcm[i]) / 32768.0f;
        }
    }
    SDL_free(data);
    return ok;
}

}

bool audio_init(Audio& audio, int volume_percent)
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Audio unavailable: %s", SDL_GetError());
        return false;
    }
    for (int i = 0; i < static_cast<int>(Sound::Count); i++) {
        load_wav(asset_path(std::string("sounds/") + sound_file_name(static_cast<Sound>(i))),
                 audio.fixed[static_cast<size_t>(i)]);
    }
    for (int lines = 1; lines <= kMaxClearLines; lines++) {
        for (int step = 0; step < kMaxClearSteps; step++) {
            load_wav(asset_path("sounds/" + clear_file_name(lines, step)),
                     audio.clears[static_cast<size_t>(lines - 1)][static_cast<size_t>(step)]);
        }
    }
    audio.mixer.volume = static_cast<float>(volume_percent) / 100.0f;
    SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, kSampleRate};
    audio.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, feed, &audio);
    if (!audio.stream) {
        SDL_Log("Cannot open audio: %s", SDL_GetError());
        return false;
    }
    SDL_ResumeAudioStreamDevice(audio.stream);
    return true;
}

void audio_shutdown(Audio& audio)
{
    if (audio.stream) {
        SDL_DestroyAudioStream(audio.stream);
        audio.stream = nullptr;
    }
}

void audio_set_volume(Audio& audio, int volume_percent)
{
    if (!audio.stream) {
        return;
    }
    SDL_LockAudioStream(audio.stream);
    audio.mixer.volume = static_cast<float>(volume_percent) / 100.0f;
    SDL_UnlockAudioStream(audio.stream);
}

void audio_play(Audio& audio, Sound sound)
{
    if (!audio.stream || audio.mixer.volume <= 0.0f) {
        return;
    }
    SDL_LockAudioStream(audio.stream);
    mixer_play(audio.mixer, audio.fixed[static_cast<size_t>(sound)], 1.0f);
    SDL_UnlockAudioStream(audio.stream);
}

void audio_play_clear(Audio& audio, int lines, int streak)
{
    if (!audio.stream || audio.mixer.volume <= 0.0f) {
        return;
    }
    size_t l = static_cast<size_t>(std::clamp(lines, 1, kMaxClearLines) - 1);
    size_t s = static_cast<size_t>(std::clamp(streak - 1, 0, kMaxClearSteps - 1));
    SDL_LockAudioStream(audio.stream);
    mixer_play(audio.mixer, audio.clears[l][s], 1.0f);
    SDL_UnlockAudioStream(audio.stream);
}
