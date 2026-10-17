// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "game.hpp"

#include <utility>

namespace {

Tray next_tray(Game& game)
{
    if (game.deal_mode == 1) {
        return deal_fitting(game.rng, game.board, game.allowed);
    }
    if (game.deal_mode == 2) {
        return deal_dynamic(game.rng, game.board, game.allowed);
    }
    return deal_tray(game.rng, game.board, game.allowed);
}

}

void game_start(Game& game, uint32_t seed, const GameOptions& options)
{
    game.rescues_left = options.rescues;
    game.deal_mode = options.deal_mode;
    game.allowed = options.allowed;
    game.board = Board{};
    game.board.size = options.size;
    game.score = ScoreState{};
    game.over = false;
    game.rng.seed(seed);
    game.hold = -1;
    game.tray = next_tray(game);
    game.over = !game_has_move(game);
}

bool game_rescue(Game& game)
{
    if (!game.over || game.rescues_left <= 0) {
        return false;
    }
    game.tray = rescue_tray(game.rng, game.board);
    if (!game_has_move(game)) {
        return false;
    }
    game.rescues_left--;
    game.over = false;
    return true;
}

bool game_has_move(const Game& game)
{
    if (any_piece_fits(game.board, game.tray)) {
        return true;
    }
    return game.hold >= 0 && fits_anywhere(game.board, piece_list()[game.hold]);
}

int game_piece_in_slot(const Game& game, int slot)
{
    if (slot == kHoldSlot) {
        return game.hold;
    }
    return slot >= 0 && slot < kTraySize ? game.tray[slot] : -1;
}

bool game_swap_hold(Game& game, int slot)
{
    if (game.over || slot < 0 || slot >= kTraySize || game.tray[slot] < 0) {
        return false;
    }
    if (game.tray[slot] >= base_piece_count() || game.hold >= base_piece_count()) {
        return false;
    }
    std::swap(game.tray[slot], game.hold);
    bool empty = true;
    for (int index : game.tray) {
        empty = empty && index < 0;
    }
    if (empty) {
        game.tray = next_tray(game);
    }
    game.over = !game_has_move(game);
    return true;
}

MoveResult game_place(Game& game, int slot, int row, int col)
{
    MoveResult result;
    if (game.over || game_piece_in_slot(game, slot) < 0) {
        return result;
    }
    const Piece& piece = piece_list()[game_piece_in_slot(game, slot)];
    if (!can_place(game.board, piece, row, col)) {
        return result;
    }
    place(game.board, piece, row, col);
    Lines lines = full_lines(game.board);
    result.lines = line_count(lines);
    result.cleared = line_cell_mask(lines, game.board.size);
    clear_lines(game.board, lines);
    result.gain = apply_move_score(game.score, static_cast<int>(piece.cells.size()), result.lines);
    result.ok = true;
    if (slot == kHoldSlot) {
        game.hold = -1;
    } else {
        game.tray[slot] = -1;
    }
    bool empty = true;
    for (int index : game.tray) {
        empty = empty && index < 0;
    }
    if (empty) {
        game.tray = next_tray(game);
    }
    game.over = !game_has_move(game);
    return result;
}
