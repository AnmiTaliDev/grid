// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <algorithm>
#include <cmath>
#include <set>

#include "bests.hpp"
#include "game.hpp"
#include "mixer.hpp"
#include "serialize.hpp"
#include "synth.hpp"

namespace {

constexpr uint64_t kAnyPiece = ~uint64_t{0};

GameOptions options(int size, int rescues, int deal_mode)
{
    GameOptions o;
    o.size = size;
    o.rescues = rescues;
    o.deal_mode = deal_mode;
    return o;
}

void test_piece_list()
{
    const std::vector<Piece>& list = piece_list();
    assert(base_piece_count() == 46);
    assert(list.size() == 46 + kCustomSlots);
    for (int i = 0; i < base_piece_count(); i++) {
        const Piece& p = list[i];
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
    assert(find_piece("plus", 0) != -1 && find_piece("plus", 1) == -1);
    assert(find_piece("cup", 3) != -1 && find_piece("cup", 4) == -1);
    assert(find_piece("diag2", 1) != -1 && find_piece("diag2", 2) == -1);
    assert(find_piece("diag3", 1) != -1 && find_piece("diag3", 2) == -1);
    assert(find_piece("stairs", 3) != -1 && find_piece("stairs", 4) == -1);
    assert(find_piece("skew5", 3) != -1 && find_piece("skew5", 4) == -1);
    const Piece& diag = list[find_piece("diag2", 0)];
    assert(diag.cells.size() == 2 && diag.rows == 2 && diag.cols == 2);
    Board board;
    assert(can_place(board, diag, 0, 0));
    board.cell[0][0] = 1;
    assert(!can_place(board, diag, 0, 0));
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
    game_start(game, 1, options(8, 1, false));
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
    game_start(game, 3, options(6, 1, false));
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
    game_start(game, 1, options(8, 1, false));
    game.board = board;
    game.tray = {dot, bar2, bar2};
    game.over = false;
    MoveResult r = game_place(game, 0, 0, 0);
    assert(r.ok && r.lines == 0);
    assert(game.over);
    MoveResult again = game_place(game, 1, 0, 0);
    assert(!again.ok);
}

void test_rescue()
{
    Board board;
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            board.cell[r][c] = 1;
        }
    }
    board.cell[0][0] = 0;
    board.cell[1][0] = 0;
    board.cell[4][4] = 0;
    board.cell[7][7] = 0;
    int vertical = find_piece("bar2", 1);
    int dot = find_piece("dot", 0);
    int bar2 = find_piece("bar2", 0);
    assert(!any_piece_fits(board, {bar2, bar2, bar2}));
    std::mt19937 rng(5);
    for (int i = 0; i < 50; i++) {
        Tray rescue = rescue_tray(rng, board);
        assert(any_piece_fits(board, rescue));
        assert(std::find(rescue.begin(), rescue.end(), vertical) != rescue.end());
        assert(std::find(rescue.begin(), rescue.end(), dot) != rescue.end());
    }
    Game game;
    game_start(game, 9, options(8, 1, false));
    game.board = board;
    game.tray = {bar2, bar2, bar2};
    game.over = true;
    assert(game_rescue(game));
    assert(!game.over && game.rescues_left == 0);
    assert(any_piece_fits(game.board, game.tray));
    game.over = true;
    assert(!game_rescue(game));
    game_start(game, 9, options(8, 0, false));
    game.over = true;
    assert(!game_rescue(game));
}

void test_fitting_deal()
{
    Board sparse = sparse_board();
    int allowed[3] = {find_piece("dot", 0), find_piece("diag2", 0), find_piece("diag3", 0)};
    std::mt19937 rng(21);
    for (int i = 0; i < 500; i++) {
        Tray t = deal_fitting(rng, sparse, kAnyPiece);
        for (int index : t) {
            assert(index == allowed[0] || index == allowed[1] || index == allowed[2]);
        }
    }
    Board empty;
    for (int i = 0; i < 200; i++) {
        Tray t = deal_fitting(rng, empty, kAnyPiece);
        for (int index : t) {
            assert(fits_anywhere(empty, piece_list()[index]));
        }
    }
    Game game;
    game_start(game, 4, options(8, 0, true));
    assert(game.deal_mode == 1);
    for (int index : game.tray) {
        assert(fits_anywhere(game.board, piece_list()[index]));
    }
}

void test_piece_filter()
{
    assert(shape_count() == 18);
    assert(all_shapes() == (1u << 18) - 1);
    assert(pieces_for_shapes(all_shapes()) == (uint64_t{1} << 46) - 1);
    uint32_t only_dot = 1u << piece_list()[find_piece("dot", 0)].shape;
    uint64_t allowed = pieces_for_shapes(only_dot);
    assert(allowed == uint64_t{1} << find_piece("dot", 0));
    uint32_t bars = (1u << piece_list()[find_piece("bar2", 0)].shape) | (1u << piece_list()[find_piece("bar3", 0)].shape);
    assert(__builtin_popcountll(pieces_for_shapes(bars)) == 4);
    Board empty;
    std::mt19937 rng(77);
    for (int i = 0; i < 300; i++) {
        for (int index : deal_tray(rng, empty, allowed)) {
            assert(index == find_piece("dot", 0));
        }
        for (int index : deal_fitting(rng, empty, pieces_for_shapes(bars))) {
            assert(piece_list()[index].name == "bar2" || piece_list()[index].name == "bar3");
        }
    }
    GameOptions o;
    o.allowed = allowed;
    Game game;
    game_start(game, 5, o);
    for (int index : game.tray) {
        assert(index == find_piece("dot", 0));
    }
    int first = first_piece_of_shape(piece_list()[find_piece("tee", 0)].shape);
    assert(first == find_piece("tee", 0));
}

void test_dynamic_deal()
{
    std::mt19937 rng(31);
    Board empty;
    int counts[46] = {};
    for (int i = 0; i < 3000; i++) {
        for (int index : deal_dynamic(rng, empty, kAnyPiece)) {
            counts[index]++;
        }
    }
    for (int count : counts) {
        assert(count > 120 && count < 280);
    }
    Board sparse = sparse_board();
    int allowed[3] = {find_piece("dot", 0), find_piece("diag2", 0), find_piece("diag3", 0)};
    for (int i = 0; i < 500; i++) {
        for (int index : deal_dynamic(rng, sparse, kAnyPiece)) {
            assert(index == allowed[0] || index == allowed[1] || index == allowed[2]);
        }
    }
    Board packed;
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            packed.cell[r][c] = 1;
        }
        packed.cell[r][(r * 3) % 8] = 0;
    }
    int dot = find_piece("dot", 0);
    for (int i = 0; i < 200; i++) {
        for (int index : deal_dynamic(rng, packed, kAnyPiece)) {
            assert(index == dot);
        }
    }
    Board half;
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 7; c++) {
            half.cell[r][c] = 1;
        }
    }
    int invented = 0;
    for (int i = 0; i < 400; i++) {
        Tray tray = deal_dynamic(rng, half, kAnyPiece);
        for (int index : tray) {
            if (index >= base_piece_count()) {
                invented++;
                const Piece& p = piece_list()[index];
                assert(fits_anywhere(half, p));
                assert(p.cells.size() >= 2 && p.cells.size() <= 6);
                assert(p.rows <= 4 && p.cols <= 4);
                for (int k = 0; k < base_piece_count(); k++) {
                    const Piece& base = piece_list()[k];
                    bool same = base.cells.size() == p.cells.size() && base.rows == p.rows && base.cols == p.cols;
                    for (size_t j = 0; same && j < p.cells.size(); j++) {
                        same = base.cells[j].row == p.cells[j].row && base.cells[j].col == p.cells[j].col;
                    }
                    assert(!same);
                }
            } else {
                assert(index >= 0);
            }
        }
    }
    std::printf("dynamic deal: %d of 1200 pieces are invented shapes\n", invented);
    assert(invented > 400);
    GameOptions only_dots;
    only_dots.allowed = pieces_for_shapes(1u << piece_list()[find_piece("dot", 0)].shape);
    only_dots.deal_mode = 2;
    Game game;
    game_start(game, 8, only_dots);
    for (int index : game.tray) {
        assert(index == find_piece("dot", 0));
    }
}

void test_save_roundtrip()
{
    Game game;
    GameOptions o = options(7, 2, 2);
    game_start(game, 11, o);
    game.board.cell[3][3] = 4;
    game.board.cell[6][0] = 1;
    game.score.score = 1234;
    game.score.streak = 3;
    int custom = register_custom_piece({{0, 0}, {0, 1}, {1, 1}, {2, 1}, {2, 2}}, 3);
    game.tray = {find_piece("tee", 1), -1, custom};
    std::string text = serialize_game(game, 777);
    Game loaded;
    loaded.rng.seed(1);
    int previous = 0;
    assert(deserialize_game(text, loaded, previous));
    assert(previous == 777);
    assert(loaded.board.size == 7);
    assert(loaded.score.score == 1234 && loaded.score.streak == 3);
    assert(loaded.rescues_left == 2 && loaded.deal_mode == 2 && !loaded.over);
    assert(loaded.board.cell[3][3] == 4 && loaded.board.cell[6][0] == 1);
    assert(loaded.tray[0] == find_piece("tee", 1) && loaded.tray[1] == -1);
    const Piece& a = piece_list()[custom];
    const Piece& b = piece_list()[loaded.tray[2]];
    assert(loaded.tray[2] >= base_piece_count());
    assert(a.glaze == b.glaze && a.cells.size() == b.cells.size() && a.rows == b.rows && a.cols == b.cols);
    for (size_t i = 0; i < a.cells.size(); i++) {
        assert(a.cells[i].row == b.cells[i].row && a.cells[i].col == b.cells[i].col);
    }
    assert(serialize_game(loaded, 777) != "");
}

void test_save_rejects_garbage()
{
    Game game;
    game_start(game, 12, options(8, 1, 0));
    std::string good = serialize_game(game, 5);
    Game out;
    int previous = 0;
    assert(deserialize_game(good, out, previous));
    assert(!deserialize_game("", out, previous));
    assert(!deserialize_game("hello world", out, previous));
    assert(!deserialize_game(good.substr(0, good.size() / 2), out, previous));
    std::string bad_version = good;
    bad_version.replace(bad_version.find(" 1 size"), 7, " 9 size");
    assert(!deserialize_game(bad_version, out, previous));
    std::string bad_cell = good;
    bad_cell.replace(bad_cell.find("cells ") + 6, 1, "9");
    assert(!deserialize_game(bad_cell, out, previous));
    Game stuck;
    game_start(stuck, 3, options(8, 0, 0));
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            stuck.board.cell[r][c] = (r + c) % 2 == 0 ? 1 : 0;
        }
    }
    stuck.tray = {find_piece("bar2", 0), find_piece("bar2", 1), -1};
    assert(!deserialize_game(serialize_game(stuck, 0), out, previous));
}

void test_bests()
{
    Bests b;
    assert(b.get(6) == 0 && b.get(8) == 0 && b.get(99) == 0 && b.get(-1) == 0);
    b.set(6, 120);
    b.set(8, 4500);
    b.set(99, 5);
    b.set(7, -3);
    assert(b.get(6) == 120 && b.get(7) == 0 && b.get(8) == 4500);
    Bests back = parse_bests(serialize_bests(b));
    assert(back.get(6) == 120 && back.get(7) == 0 && back.get(8) == 4500);
    Bests legacy = parse_bests("3240\n");
    assert(legacy.get(8) == 3240 && legacy.get(6) == 0);
    assert(parse_bests("").get(8) == 0);
    assert(parse_bests("garbage").get(8) == 0);
    Bests partial = parse_bests("bests 6 50 7");
    assert(partial.get(6) == 50 && partial.get(7) == 0);
    Bests range = parse_bests("bests 1 9 6 -4 9 9 8 77");
    assert(range.get(1) == 0 && range.get(6) == 0 && range.get(8) == 77);
}

void test_hold()
{
    Game game;
    game_start(game, 14, options(8, 0, 0));
    int dot = find_piece("dot", 0);
    int bar2 = find_piece("bar2", 0);
    int bar3 = find_piece("bar3", 0);
    game.tray = {dot, bar2, bar3};
    assert(game.hold == -1);
    assert(game_swap_hold(game, 1));
    assert(game.hold == bar2 && game.tray[1] == -1);
    assert(game_swap_hold(game, 0));
    assert(game.hold == dot && game.tray[0] == bar2 && game.tray[1] == -1);
    assert(!game_swap_hold(game, 1));
    assert(game_piece_in_slot(game, kHoldSlot) == dot);
    MoveResult r = game_place(game, kHoldSlot, 2, 2);
    assert(r.ok && game.hold == -1 && game.board.cell[2][2] != 0);
    assert(game.tray[0] == bar2 && game.tray[2] == bar3);
    assert(!game_place(game, kHoldSlot, 0, 0).ok);
    int custom = register_custom_piece({{0, 0}, {1, 0}, {1, 1}}, 2);
    game.tray = {custom, bar2, -1};
    assert(!game_swap_hold(game, 0));
    assert(game_swap_hold(game, 1));
    assert(game.hold == bar2);
    assert(!game_swap_hold(game, 0));
    game.tray = {-1, -1, bar3};
    game.hold = dot;
    assert(game_swap_hold(game, 2));
    assert(game.hold == bar3 && game.tray[2] == dot);
    game.tray = {-1, -1, dot};
    game.hold = -1;
    assert(game_swap_hold(game, 2));
    bool any = false;
    for (int index : game.tray) {
        any = any || index >= 0;
    }
    assert(any && game.hold == dot);
}

void test_hold_ends_game_correctly()
{
    Game game;
    game_start(game, 15, options(8, 0, 0));
    Board sparse = sparse_board();
    int dot = find_piece("dot", 0);
    int bar2 = find_piece("bar2", 0);
    game.board = sparse;
    game.tray = {bar2, -1, -1};
    game.hold = -1;
    assert(!game_has_move(game));
    game.hold = dot;
    assert(game_has_move(game));
    game.over = false;
    Game stuck = game;
    stuck.hold = bar2;
    assert(!game_has_move(stuck));
    game.hold = dot;
    std::string text = serialize_game(game, 3);
    Game loaded;
    int previous = 0;
    assert(deserialize_game(text, loaded, previous));
    assert(loaded.hold == dot);
    Game fresh;
    game_start(fresh, 16, options(8, 0, 0));
    std::string full = serialize_game(fresh, 3);
    std::string legacy = full.substr(0, full.find(" hold"));
    Game old;
    assert(deserialize_game(legacy + "\n", old, previous));
    assert(old.hold == -1);
    std::string bad = full;
    bad.replace(bad.find(" hold -"), 7, " hold x");
    assert(!deserialize_game(bad, old, previous));
}

void test_synth_and_mixer()
{
    for (int i = 0; i < static_cast<int>(Sound::Count); i++) {
        std::vector<float> s = synth_sound(static_cast<Sound>(i));
        assert(s.size() > kSampleRate / 50 && s.size() < kSampleRate * 2);
        float peak = 0;
        for (float v : s) {
            assert(std::isfinite(v) && v >= -1.0f && v <= 1.0f);
            peak = std::max(peak, std::fabs(v));
        }
        assert(peak > 0.05f);
        assert(std::fabs(s.back()) < 0.02f);
    }
    for (int lines = 1; lines <= kMaxClearLines; lines++) {
        for (int streak = 0; streak <= kMaxClearSteps + 2; streak++) {
            std::vector<float> s = synth_clear(lines, streak);
            assert(s.size() > kSampleRate / 10);
            for (float v : s) {
                assert(std::isfinite(v) && v >= -1.0f && v <= 1.0f);
            }
        }
    }
    assert(synth_clear(4, 1).size() > synth_clear(1, 1).size());

    std::vector<float> ones(100, 0.5f);
    std::vector<float> shorter(40, 0.25f);
    Mixer m;
    m.volume = 1.0f;
    mixer_play(m, ones, 1.0f);
    mixer_play(m, shorter, 1.0f);
    std::vector<float> out(60);
    mixer_render(m, out.data(), 60);
    assert(std::fabs(out[0] - 0.75f) < 1e-6f && std::fabs(out[39] - 0.75f) < 1e-6f);
    assert(std::fabs(out[40] - 0.5f) < 1e-6f && std::fabs(out[59] - 0.5f) < 1e-6f);
    assert(m.voices.size() == 1);
    mixer_render(m, out.data(), 60);
    assert(std::fabs(out[39] - 0.5f) < 1e-6f && std::fabs(out[40]) < 1e-6f);
    assert(m.voices.empty());
    mixer_render(m, out.data(), 60);
    assert(out[0] == 0.0f);
    m.volume = 0.5f;
    mixer_play(m, ones, 1.0f);
    mixer_render(m, out.data(), 10);
    assert(std::fabs(out[0] - 0.25f) < 1e-6f);
    std::vector<float> loud(10, 1.0f);
    m.volume = 1.0f;
    m.voices.clear();
    mixer_play(m, loud, 1.0f);
    mixer_play(m, loud, 1.0f);
    mixer_render(m, out.data(), 10);
    assert(out[0] == 1.0f);
    for (size_t i = 0; i < kMaxVoices + 5; i++) {
        mixer_play(m, loud, 1.0f);
    }
    assert(m.voices.size() == kMaxVoices);
}

void test_tray_refill()
{
    Game game;
    game_start(game, 7, options(8, 1, false));
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
    int counts[46] = {};
    for (int i = 0; i < 3000; i++) {
        Tray t = deal_tray(rng, empty, kAnyPiece);
        for (int index : t) {
            counts[index]++;
        }
    }
    for (int count : counts) {
        assert(count > 120 && count < 280);
    }
    Board sparse = sparse_board();
    int fitting = 0;
    const int deals = 3000;
    for (int i = 0; i < deals; i++) {
        Tray t = deal_tray(rng, sparse, kAnyPiece);
        if (any_piece_fits(sparse, t)) {
            fitting++;
        }
    }
    double share = static_cast<double>(fitting) / deals;
    std::printf("deal on sparse board: %.3f of deals contain a fitting piece\n", share);
    assert(share > 0.95);
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
    test_rescue();
    test_fitting_deal();
    test_piece_filter();
    test_dynamic_deal();
    test_save_roundtrip();
    test_save_rejects_garbage();
    test_bests();
    test_hold();
    test_hold_ends_game_correctly();
    test_synth_and_mixer();
    test_tray_refill();
    test_deal_fairness();
    test_score_and_streak();
    std::puts("logic tests passed");
    return 0;
}
