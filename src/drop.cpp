// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "drop.hpp"

#include <cmath>

#include "layout.hpp"

Point piece_size_on_board(const Piece& piece)
{
    return {kCellPitch * static_cast<float>(piece.cols) - 4.0f, kCellPitch * static_cast<float>(piece.rows) - 4.0f};
}

Point footprint_origin(const Piece& piece, float pointer_x, float pointer_y)
{
    Point size = piece_size_on_board(piece);
    return {pointer_x - size.x / 2, pointer_y - size.y / 2};
}

Drop find_drop(const Board& board, const Piece& piece, float pointer_x, float pointer_y)
{
    Point o = footprint_origin(piece, pointer_x, pointer_y);
    Drop d;
    d.col = static_cast<int>(std::floor((o.x - kBoardX) / kCellPitch + 0.5f));
    d.row = static_cast<int>(std::floor((o.y - kBoardY) / kCellPitch + 0.5f));
    d.valid = can_place(board, piece, d.row, d.col);
    return d;
}
