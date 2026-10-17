// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "score.hpp"

#include <algorithm>

int apply_move_score(ScoreState& state, int piece_cells, int lines)
{
    if (lines > 0) {
        state.streak++;
        state.idle = 0;
    } else {
        state.idle++;
        if (state.idle >= kStreakGraceMoves) {
            state.streak = 0;
            state.idle = 0;
        }
    }
    int gain = piece_cells;
    if (lines > 0) {
        gain += kClearBase * lines * lines * std::max(1, state.streak);
    }
    state.score += gain;
    return gain;
}
