// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "stats.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "kv.hpp"

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
    return write_kv({
        {"games", std::to_string(stats.games)},
        {"total", std::to_string(stats.total_score)},
        {"streak", std::to_string(stats.best_streak)},
        {"lines", std::to_string(stats.lines)},
        {"pieces", std::to_string(stats.pieces)},
        {"seconds", std::to_string(stats.seconds)},
    });
}

Stats parse_stats(const std::string& text)
{
    Kv kv;
    if (text.find('=') != std::string::npos) {
        kv = parse_kv(text);
    } else {
        std::vector<std::string> words = split_words(text);
        if (words.empty() || words[0] != "stats") {
            return Stats{};
        }
        kv = parse_word_pairs(text, true);
    }
    const int64_t big = 2000000000;
    Stats s;
    s.games = static_cast<int>(std::min<int64_t>(kv.get_int("games", 0, 0, INT64_MAX), big));
    s.total_score = kv.get_int("total", 0, 0, INT64_MAX);
    s.best_streak = static_cast<int>(std::min<int64_t>(kv.get_int("streak", 0, 0, INT64_MAX), big));
    s.lines = static_cast<int>(std::min<int64_t>(kv.get_int("lines", 0, 0, INT64_MAX), big));
    s.pieces = static_cast<int>(std::min<int64_t>(kv.get_int("pieces", 0, 0, INT64_MAX), big));
    s.seconds = kv.get_int("seconds", 0, 0, INT64_MAX);
    return s;
}
