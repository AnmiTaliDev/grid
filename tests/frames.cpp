// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "frames.hpp"

#include <cstring>

namespace {

void load_table(const char* const rows[8], Board& board, uint64_t* bone)
{
    for (int r = 0; r < kBoardSize; r++) {
        for (int c = 0; c < kBoardSize; c++) {
            char ch = rows[r][c * 2];
            int value = 0;
            if (ch >= 'a' && ch <= 'e') {
                value = ch - 'a' + 1;
            } else if (ch == 'w' && bone) {
                *bone |= cell_bit(r, c);
            }
            board.cell[r][c] = value;
        }
    }
}

Tray reference_tray()
{
    return {find_piece("l4", 0), -1, find_piece("zigzag", 0)};
}

}

MenuView reference_menu()
{
    MenuView v;
    v.best = 12480;
    return v;
}

GameView reference_game()
{
    const char* const rows[8] = {
        "n n n n n n n n", "n n n n n n n n", "n n n n n n n n", "n n n n n n n n",
        "a n c c n n n d", "a b c n n n n d", "a b e n n n a d", "a b e g g g a a",
    };
    GameView v;
    load_table(rows, v.board, nullptr);
    v.score = 3240;
    v.best = 12480;
    v.streak = 3;
    v.tray = reference_tray();
    v.tray[1] = find_piece("bar3", 0);
    v.hand_slot = 1;
    v.pointer_x = 222 + 170.0f / 2;
    v.pointer_y = 534 + 54.0f / 2;
    return v;
}

GameView reference_clear()
{
    const char* const rows[8] = {
        "n n n n n n n w", "n n n n n n n w", "n n n n n n n w", "n n n n n n n w",
        "a n c c n n n w", "a b c d d b b w", "a b e d n n b w", "w w w w w w w w",
    };
    GameView v;
    load_table(rows, v.board, &v.bone_mask);
    v.score = 3420;
    v.best = 12480;
    v.streak = 4;
    v.tray = reference_tray();
    v.popup = true;
    v.popup_gain = 180;
    v.popup_lines = 2;
    return v;
}

OverView reference_over()
{
    const char* const rows[8] = {
        "a a n b b b e e", "a n c c n b e d", "n e c d d n n d", "e e n d a a b b",
        "d n c c a n b n", "d d c n e e b a", "n b b e e n d a", "c n a a n d d n",
    };
    OverView v;
    load_table(rows, v.board, nullptr);
    v.score = 14320;
    v.previous_best = 12480;
    v.new_best = true;
    return v;
}
