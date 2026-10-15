// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "game.hpp"

void game_start(Game& game, uint32_t seed)
{
    game.board = Board{};
    game.score = ScoreState{};
    game.over = false;
    game.rng.seed(seed);
    game.tray = deal_tray(game.rng, game.board);
    game.over = !any_piece_fits(game.board, game.tray);
}

MoveResult game_place(Game& game, int slot, int row, int col)
{
    MoveResult result;
    if (game.over || slot < 0 || slot >= kTraySize || game.tray[slot] < 0) {
        return result;
    }
    const Piece& piece = piece_list()[game.tray[slot]];
    if (!can_place(game.board, piece, row, col)) {
        return result;
    }
    place(game.board, piece, row, col);
    Lines lines = full_lines(game.board);
    result.lines = line_count(lines);
    result.cleared = line_cell_mask(lines);
    clear_lines(game.board, lines);
    result.gain = apply_move_score(game.score, static_cast<int>(piece.cells.size()), result.lines);
    result.ok = true;
    game.tray[slot] = -1;
    bool empty = true;
    for (int index : game.tray) {
        empty = empty && index < 0;
    }
    if (empty) {
        game.tray = deal_tray(game.rng, game.board);
    }
    game.over = !any_piece_fits(game.board, game.tray);
    return result;
}
