// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "score.hpp"

#include <algorithm>

int apply_move_score(ScoreState& state, int piece_cells, int lines)
{
    state.streak = lines > 0 ? state.streak + 1 : 0;
    int gain = piece_cells;
    if (lines > 0) {
        gain += 10 * lines * lines * std::max(1, state.streak);
    }
    state.score += gain;
    return gain;
}
