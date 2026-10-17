// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

constexpr int kClearBase = 50;
constexpr int kStreakGraceMoves = 3;

struct ScoreState {
    int score = 0;
    int streak = 0;
    int idle = 0;
};

int apply_move_score(ScoreState& state, int piece_cells, int lines);
