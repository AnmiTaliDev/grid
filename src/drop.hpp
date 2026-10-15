// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "board.hpp"

struct Point {
    float x;
    float y;
};

struct Drop {
    bool valid = false;
    int row = 0;
    int col = 0;
};

Point piece_size_on_board(const Piece& piece);
Point footprint_origin(const Piece& piece, float pointer_x, float pointer_y);
Drop find_drop(const Board& board, const Piece& piece, float pointer_x, float pointer_y);
