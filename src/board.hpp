// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <array>
#include <cstdint>

#include "piece.hpp"

constexpr int kBoardSize = 8;

struct Board {
    int size = kBoardSize;
    std::array<std::array<int, kBoardSize>, kBoardSize> cell{};
};

struct Lines {
    uint8_t rows = 0;
    uint8_t cols = 0;
};

int line_count(Lines lines);
uint64_t line_cell_mask(Lines lines, int size);
uint64_t cell_bit(int row, int col);

bool can_place(const Board& board, const Piece& piece, int row, int col);
void place(Board& board, const Piece& piece, int row, int col);
Lines full_lines(const Board& board);
void clear_lines(Board& board, Lines lines);
bool fits_anywhere(const Board& board, const Piece& piece);
