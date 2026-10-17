// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <string>

struct Stats {
    int games = 0;
    int64_t total_score = 0;
    int best_streak = 0;
    int lines = 0;
    int pieces = 0;
    int64_t seconds = 0;
};

void stats_on_move(Stats& stats, int lines, int streak);
void stats_on_game_end(Stats& stats, int score);
void stats_revert_game_end(Stats& stats, int score);
int stats_average(const Stats& stats);
std::string serialize_stats(const Stats& stats);
Stats parse_stats(const std::string& text);
