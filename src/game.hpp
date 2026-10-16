// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <random>

#include "board.hpp"
#include "deal.hpp"
#include "score.hpp"

struct Game {
    Board board;
    Tray tray{-1, -1, -1};
    ScoreState score;
    bool over = false;
    int rescues_left = 0;
    std::mt19937 rng;
};

struct MoveResult {
    bool ok = false;
    int gain = 0;
    int lines = 0;
    uint64_t cleared = 0;
};

void game_start(Game& game, uint32_t seed, int size, int rescues);
bool game_rescue(Game& game);
MoveResult game_place(Game& game, int slot, int row, int col);
