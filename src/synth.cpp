// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "synth.hpp"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979f;

enum class Wave {
    Sine,
    Triangle,
    Square,
};

struct Note {
    float freq;
    float end_freq;
    float start_ms;
    float length_ms;
    float gain;
    Wave wave;
};

float wave_value(Wave wave, float phase)
{
    float x = phase - std::floor(phase);
    if (wave == Wave::Sine) {
        return std::sin(2 * kPi * x);
    }
    if (wave == Wave::Triangle) {
        return 4 * std::fabs(x - 0.5f) - 1;
    }
    return x < 0.5f ? 0.6f : -0.6f;
}

void add_note(std::vector<float>& buffer, const Note& n)
{
    int start = static_cast<int>(n.start_ms * kSampleRate / 1000);
    int length = static_cast<int>(n.length_ms * kSampleRate / 1000);
    int attack = std::max(1, kSampleRate / 500);
    float phase = 0;
    for (int i = 0; i < length; i++) {
        int at = start + i;
        if (at >= static_cast<int>(buffer.size())) {
            break;
        }
        float t = static_cast<float>(i) / static_cast<float>(length);
        float freq = n.freq + (n.end_freq - n.freq) * t;
        phase += freq / kSampleRate;
        float attack_gain = std::min(1.0f, static_cast<float>(i) / static_cast<float>(attack));
        float decay = std::exp(-4.5f * t) * (1.0f - t);
        buffer[at] += wave_value(n.wave, phase) * n.gain * attack_gain * decay;
    }
}

std::vector<float> render_notes(const std::vector<Note>& notes)
{
    float end = 0;
    for (const Note& n : notes) {
        end = std::max(end, n.start_ms + n.length_ms);
    }
    std::vector<float> buffer(static_cast<size_t>(end * kSampleRate / 1000) + 1, 0.0f);
    for (const Note& n : notes) {
        add_note(buffer, n);
    }
    for (float& s : buffer) {
        s = std::clamp(s, -1.0f, 1.0f);
    }
    return buffer;
}

float semitones(float base, float steps)
{
    return base * std::pow(2.0f, steps / 12.0f);
}

}

std::vector<float> synth_sound(Sound sound)
{
    switch (sound) {
    case Sound::Pick:
        return render_notes({{660, 880, 0, 45, 0.35f, Wave::Triangle}});
    case Sound::Place:
        return render_notes({{190, 90, 0, 140, 0.7f, Wave::Sine}, {1200, 600, 0, 18, 0.25f, Wave::Triangle}});
    case Sound::Invalid:
        return render_notes({{220, 150, 0, 110, 0.4f, Wave::Square}});
    case Sound::Hold:
        return render_notes({{520, 520, 0, 60, 0.35f, Wave::Triangle}, {780, 780, 55, 80, 0.35f, Wave::Triangle}});
    case Sound::Undo:
        return render_notes({{700, 450, 0, 80, 0.35f, Wave::Triangle}, {450, 300, 70, 90, 0.35f, Wave::Triangle}});
    case Sound::GameOver:
        return render_notes({{392, 392, 0, 260, 0.4f, Wave::Triangle},
                             {330, 330, 220, 260, 0.4f, Wave::Triangle},
                             {262, 262, 440, 260, 0.4f, Wave::Triangle},
                             {196, 180, 660, 520, 0.45f, Wave::Triangle}});
    case Sound::Rescue:
        return render_notes({{330, 660, 0, 150, 0.35f, Wave::Triangle},
                             {440, 880, 110, 150, 0.35f, Wave::Triangle},
                             {660, 1320, 220, 260, 0.35f, Wave::Triangle}});
    case Sound::Click:
    case Sound::Count:
        break;
    }
    return render_notes({{900, 700, 0, 30, 0.3f, Wave::Triangle}});
}

std::vector<float> synth_clear(int lines, int streak)
{
    int count = std::clamp(lines, 1, kMaxClearLines) + 2;
    int step = std::clamp(streak - 1, 0, kMaxClearSteps - 1);
    const float scale[] = {0, 2, 4, 7, 9, 12};
    std::vector<Note> notes;
    notes.push_back({190, 90, 0, 120, 0.5f, Wave::Sine});
    for (int i = 0; i < count; i++) {
        float freq = semitones(523.25f, scale[i % 6] + 12.0f * static_cast<float>(i / 6) + static_cast<float>(step));
        notes.push_back({freq, freq, 30.0f + 70.0f * static_cast<float>(i), 190, 0.32f, Wave::Triangle});
    }
    return render_notes(notes);
}
