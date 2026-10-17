// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <random>

#include "board.hpp"
#include "deal.hpp"
#include "score.hpp"

struct GameOptions {
    int size = 8;
    int rescues = 0;
    int deal_mode = 0;
    uint64_t allowed = ~uint64_t{0};
};

struct Game {
    Board board;
    Tray tray{-1, -1, -1};
    int hold = -1;
    ScoreState score;
    bool over = false;
    int rescues_left = 0;
    int deal_mode = 0;
    uint64_t allowed = ~uint64_t{0};
    std::mt19937 rng;
};

struct MoveResult {
    bool ok = false;
    int gain = 0;
    int lines = 0;
    uint64_t cleared = 0;
};

constexpr int kHoldSlot = kTraySize;

bool game_has_move(const Game& game);
int game_piece_in_slot(const Game& game, int slot);
bool game_swap_hold(Game& game, int slot);
void game_start(Game& game, uint32_t seed, const GameOptions& options);
bool game_rescue(Game& game);
MoveResult game_place(Game& game, int slot, int row, int col);
