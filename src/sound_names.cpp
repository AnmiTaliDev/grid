// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "synth.hpp"

const char* sound_file_name(Sound sound)
{
    switch (sound) {
    case Sound::Pick:
        return "pick.wav";
    case Sound::Place:
        return "place.wav";
    case Sound::Invalid:
        return "invalid.wav";
    case Sound::Hold:
        return "hold.wav";
    case Sound::Undo:
        return "undo.wav";
    case Sound::GameOver:
        return "game_over.wav";
    case Sound::Rescue:
        return "rescue.wav";
    case Sound::Click:
    case Sound::Count:
        break;
    }
    return "click.wav";
}

std::string clear_file_name(int lines, int streak_step)
{
    return "clear_" + std::to_string(lines) + "_" + std::to_string(streak_step + 1) + ".wav";
}
