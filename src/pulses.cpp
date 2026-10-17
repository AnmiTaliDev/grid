// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "pulses.hpp"

#include <algorithm>

Pulse pulse_for(Sound sound)
{
    switch (sound) {
    case Sound::Pick:
        return {6, 60};
    case Sound::Place:
        return {15, 110};
    case Sound::Invalid:
        return {25, 90};
    case Sound::Hold:
        return {15, 100};
    case Sound::Undo:
        return {15, 100};
    case Sound::GameOver:
        return {150, 200};
    case Sound::Rescue:
        return {50, 160};
    case Sound::Click:
    case Sound::Count:
        break;
    }
    return {8, 70};
}

Pulse pulse_for_clear(int lines, int streak)
{
    int n = std::clamp(lines, 1, kMaxClearLines);
    int s = std::clamp(streak - 1, 0, kMaxClearSteps - 1);
    return {30 + 15 * n, std::min(255, 150 + 20 * s + 10 * n)};
}
