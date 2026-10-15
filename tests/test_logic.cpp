// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <set>

#include "game.hpp"

namespace {

void test_piece_list()
{
    const std::vector<Piece>& list = piece_list();
    assert(list.size() == 29);
    for (const Piece& p : list) {
        int min_row = 99;
        int min_col = 99;
        for (const Cell& c : p.cells) {
            min_row = std::min(min_row, c.row);
            min_col = std::min(min_col, c.col);
            assert(c.row < p.rows && c.col < p.cols);
        }
        assert(min_row == 0 && min_col == 0);
    }
    const Piece& l4 = list[find_piece("l4", 0)];
    assert(l4.rows == 3 && l4.cols == 2);
    const Piece& zig = list[find_piece("zigzag", 0)];
    assert(zig.rows == 2 && zig.cols == 3);
    assert(find_piece("square2", 1) == -1);
    assert(find_piece("tee", 3) != -1);
}

void test_placement_validity()
{
    Board board;
    const Piece& bar3 = piece_list()[find_piece("bar3", 0)];
    assert(can_place(board, bar3, 0, 0));
    assert(can_place(board, bar3, 7, 5));
    assert(!can_place(board, bar3, 7, 6));
    assert(!can_place(board, bar3, -1, 0));
    assert(!can_place(board, bar3, 8, 0));
    board.cell[4][2] = 1;
    assert(!can_place(board, bar3, 4, 0));
    assert(!can_place(board, bar3, 4, 2));
    assert(can_place(board, bar3, 4, 3));
    place(board, bar3, 4, 3);
    assert(board.cell[4][3] == kCeladon + 1 && board.cell[4][5] == kCeladon + 1);
}

void test_simultaneous_clear()
{
    Game game;
    game_start(game, 1, 8);
    int dot = find_piece("dot", 0);
    game.tray = {dot, dot, dot};
    for (int i = 1; i < kBoardSize; i++) {
        game.board.cell[0][i] = 1;
        game.board.cell[i][0] = 2;
    }
    MoveResult r = game_place(game, 0, 0, 0);
    assert(r.ok);
    assert(r.lines == 2);
    assert(__builtin_popcountll(r.cleared) == 15);
    for (int i = 0; i < kBoardSize; i++) {
        assert(game.board.cell[0][i] == 0 && game.board.cell[i][0] == 0);
    }
    assert(game.score.score == 1 + 10 * 2 * 2 * 1);
}

void test_smaller_grid()
{
    Game game;
    game_start(game, 3, 6);
    assert(game.board.size == 6);
    const Piece& bar3 = piece_list()[find_piece("bar3", 0)];
    assert(can_place(game.board, bar3, 0, 3));
    assert(!can_place(game.board, bar3, 0, 4));
    assert(!can_place(game.board, bar3, 6, 0));
    for (int c = 1; c < 6; c++) {
        game.board.cell[2][c] = 1;
    }
    int dot = find_piece("dot", 0);
    game.tray = {dot, dot, dot};
    MoveResult r = game_place(game, 0, 2, 0);
    assert(r.ok && r.lines == 1);
    assert(__builtin_popcountll(r.cleared) == 6);
    assert(game.board.cell[2][0] == 0 && game.board.cell[2][5] == 0);
}

void test_row_only_and_untouched()
{
    Board board;
    for (int c = 0; c < kBoardSize; c++) {
        board.cell[3][c] = 1;
    }
    board.cell[5][5] = 4;
    Lines lines = full_lines(board);
    assert(lines.rows == (1 << 3) && lines.cols == 0);
    clear_lines(board, lines);
    assert(board.cell[3][0] == 0 && board.cell[5][5] == 4);
}

Board sparse_board()
{
    Board board;
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            board.cell[r][c] = 1;
        }
        board.cell[r][r] = 0;
        board.cell[r][(r + 4) % 8] = 0;
    }
    return board;
}

void test_game_over()
{
    Board board = sparse_board();
    int dot = find_piece("dot", 0);
    int bar2 = find_piece("bar2", 0);
    assert(fits_anywhere(board, piece_list()[dot]));
    assert(!fits_anywhere(board, piece_list()[bar2]));
    assert(any_piece_fits(board, {bar2, dot, -1}));
    assert(!any_piece_fits(board, {bar2, bar2, -1}));
    Game game;
    game_start(game, 1, 8);
    game.board = board;
    game.tray = {dot, bar2, bar2};
    game.over = false;
    MoveResult r = game_place(game, 0, 0, 0);
    assert(r.ok && r.lines == 0);
    assert(game.over);
    MoveResult again = game_place(game, 1, 0, 0);
    assert(!again.ok);
}

void test_tray_refill()
{
    Game game;
    game_start(game, 7, 8);
    int dot = find_piece("dot", 0);
    game.tray = {dot, -1, -1};
    MoveResult r = game_place(game, 0, 0, 0);
    assert(r.ok);
    for (int index : game.tray) {
        assert(index >= 0);
    }
}

void test_deal_fairness()
{
    std::mt19937 rng(12345);
    Board empty;
    int counts[29] = {};
    for (int i = 0; i < 3000; i++) {
        Tray t = deal_tray(rng, empty);
        for (int index : t) {
            counts[index]++;
        }
    }
    for (int count : counts) {
        assert(count > 200 && count < 430);
    }
    Board sparse = sparse_board();
    int fitting = 0;
    const int deals = 3000;
    for (int i = 0; i < deals; i++) {
        Tray t = deal_tray(rng, sparse);
        if (any_piece_fits(sparse, t)) {
            fitting++;
        }
    }
    double share = static_cast<double>(fitting) / deals;
    std::printf("deal on sparse board: %.3f of deals contain a fitting piece\n", share);
    assert(share > 0.80 && share < 0.95);
}

void test_score_and_streak()
{
    ScoreState s;
    assert(apply_move_score(s, 5, 0) == 5);
    assert(s.streak == 0);
    assert(apply_move_score(s, 1, 1) == 1 + 10);
    assert(s.streak == 1);
    assert(apply_move_score(s, 1, 1) == 1 + 20);
    assert(s.streak == 2);
    assert(apply_move_score(s, 3, 2) == 3 + 10 * 4 * 3);
    assert(s.streak == 3);
    assert(apply_move_score(s, 2, 0) == 2);
    assert(s.streak == 0);
    assert(apply_move_score(s, 1, 3) == 1 + 90);
    assert(s.score == 5 + 11 + 21 + 123 + 2 + 91);
}

}

int main()
{
    test_piece_list();
    test_placement_validity();
    test_simultaneous_clear();
    test_smaller_grid();
    test_row_only_and_untouched();
    test_game_over();
    test_tray_refill();
    test_deal_fairness();
    test_score_and_streak();
    std::puts("logic tests passed");
    return 0;
}
