// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "tutorial.hpp"

#include <algorithm>
#include <cmath>

std::vector<std::string> tutorial_caption(int page, Lang lang)
{
    const Str first[kTutorialPages] = {Str::Tutorial11, Str::Tutorial21, Str::Tutorial31};
    size_t base = static_cast<size_t>(first[std::clamp(page, 0, kTutorialPages - 1)]);
    std::vector<std::string> lines;
    for (size_t i = 0; i < 3; i++) {
        lines.push_back(tr(lang, static_cast<Str>(base + i)));
    }
    return lines;
}

float tutorial_loop(float seconds)
{
    if (seconds < 0) {
        return 0;
    }
    return std::fmod(seconds, kTutorialPeriod);
}

float tutorial_ease(float t, float start, float end)
{
    if (end <= start) {
        return t >= end ? 1.0f : 0.0f;
    }
    float x = std::clamp((t - start) / (end - start), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}
