// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "stats.hpp"

#include <algorithm>
#include <sstream>

void stats_on_move(Stats& stats, int lines, int streak)
{
    stats.pieces++;
    stats.lines += std::max(0, lines);
    stats.best_streak = std::max(stats.best_streak, streak);
}

void stats_on_game_end(Stats& stats, int score)
{
    stats.games++;
    stats.total_score += std::max(0, score);
}

void stats_revert_game_end(Stats& stats, int score)
{
    if (stats.games > 0) {
        stats.games--;
        stats.total_score = std::max<int64_t>(0, stats.total_score - std::max(0, score));
    }
}

int stats_average(const Stats& stats)
{
    if (stats.games <= 0) {
        return 0;
    }
    return static_cast<int>(std::min<int64_t>(stats.total_score / stats.games, 2000000000));
}

std::string serialize_stats(const Stats& stats)
{
    std::ostringstream out;
    out << "stats games " << stats.games << " total " << stats.total_score << " streak " << stats.best_streak
        << " lines " << stats.lines << " pieces " << stats.pieces << " seconds " << stats.seconds << "\n";
    return out.str();
}

Stats parse_stats(const std::string& text)
{
    Stats s;
    std::istringstream in(text);
    std::string tag;
    if (!(in >> tag) || tag != "stats") {
        return s;
    }
    std::string key;
    int64_t value = 0;
    while (in >> key >> value) {
        if (value < 0) {
            continue;
        }
        if (key == "games") {
            s.games = static_cast<int>(std::min<int64_t>(value, 2000000000));
        } else if (key == "total") {
            s.total_score = value;
        } else if (key == "streak") {
            s.best_streak = static_cast<int>(std::min<int64_t>(value, 2000000000));
        } else if (key == "lines") {
            s.lines = static_cast<int>(std::min<int64_t>(value, 2000000000));
        } else if (key == "pieces") {
            s.pieces = static_cast<int>(std::min<int64_t>(value, 2000000000));
        } else if (key == "seconds") {
            s.seconds = value;
        }
    }
    return s;
}
