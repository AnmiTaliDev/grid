// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "board.hpp"

#include <bit>

uint64_t cell_bit(int row, int col)
{
    return uint64_t{1} << (row * kBoardSize + col);
}

int line_count(Lines lines)
{
    return std::popcount(lines.rows) + std::popcount(lines.cols);
}

uint64_t line_cell_mask(Lines lines)
{
    uint64_t mask = 0;
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            if ((lines.rows >> r & 1) || (lines.cols >> c & 1)) {
                mask |= cell_bit(r, c);
            }
        }
    }
    return mask;
}

bool can_place(const Board& board, const Piece& piece, int row, int col)
{
    for (const Cell& c : piece.cells) {
        int r = row + c.row;
        int k = col + c.col;
        if (r < 0 || r >= kBoardSize || k < 0 || k >= kBoardSize || board.cell[r][k] != 0) {
            return false;
        }
    }
    return true;
}

void place(Board& board, const Piece& piece, int row, int col)
{
    for (const Cell& c : piece.cells) {
        board.cell[row + c.row][col + c.col] = piece.glaze + 1;
    }
}

Lines full_lines(const Board& board)
{
    Lines lines;
    for (int r = 0; r < kBoardSize; r++) {
        bool full = true;
        for (int c = 0; c < kBoardSize; c++) {
            full = full && board.cell[r][c] != 0;
        }
        if (full) {
            lines.rows |= static_cast<uint8_t>(1 << r);
        }
    }
    for (int c = 0; c < kBoardSize; c++) {
        bool full = true;
        for (int r = 0; r < kBoardSize; r++) {
            full = full && board.cell[r][c] != 0;
        }
        if (full) {
            lines.cols |= static_cast<uint8_t>(1 << c);
        }
    }
    return lines;
}

void clear_lines(Board& board, Lines lines)
{
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            if ((lines.rows >> r & 1) || (lines.cols >> c & 1)) {
                board.cell[r][c] = 0;
            }
        }
    }
}

bool fits_anywhere(const Board& board, const Piece& piece)
{
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            if (can_place(board, piece, r, c)) {
                return true;
            }
        }
    }
    return false;
}
