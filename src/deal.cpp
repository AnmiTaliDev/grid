// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "deal.hpp"

bool any_piece_fits(const Board& board, const Tray& tray)
{
    const std::vector<Piece>& list = piece_list();
    for (int index : tray) {
        if (index >= 0 && fits_anywhere(board, list[index])) {
            return true;
        }
    }
    return false;
}

Tray deal_tray(std::mt19937& rng, const Board& board)
{
    std::uniform_int_distribution<int> pick(0, static_cast<int>(piece_list().size()) - 1);
    Tray tray{};
    for (int attempt = 0; attempt < kDealTries; attempt++) {
        for (int& slot : tray) {
            slot = pick(rng);
        }
        if (any_piece_fits(board, tray)) {
            return tray;
        }
    }
    return tray;
}
