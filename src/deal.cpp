// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "deal.hpp"

#include <algorithm>
#include <vector>

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

namespace {

std::vector<int> label_regions(const Board& board, std::vector<int>& sizes)
{
    int n = board.size;
    std::vector<int> label(static_cast<size_t>(n) * n, -1);
    for (int start = 0; start < n * n; start++) {
        if (label[start] != -1 || board.cell[start / n][start % n] != 0) {
            continue;
        }
        int id = static_cast<int>(sizes.size());
        sizes.push_back(0);
        std::vector<int> stack{start};
        label[start] = id;
        while (!stack.empty()) {
            int at = stack.back();
            stack.pop_back();
            sizes[id]++;
            int r = at / n;
            int c = at % n;
            const int dr[4] = {-1, 1, 0, 0};
            const int dc[4] = {0, 0, -1, 1};
            for (int k = 0; k < 4; k++) {
                int nr = r + dr[k];
                int nc = c + dc[k];
                if (nr < 0 || nc < 0 || nr >= n || nc >= n) {
                    continue;
                }
                int next = nr * n + nc;
                if (label[next] == -1 && board.cell[nr][nc] == 0) {
                    label[next] = id;
                    stack.push_back(next);
                }
            }
        }
    }
    return label;
}

}

Tray rescue_tray(std::mt19937& rng, const Board& board)
{
    const std::vector<Piece>& list = piece_list();
    std::vector<int> sizes;
    std::vector<int> label = label_regions(board, sizes);
    std::vector<int> exact;
    std::vector<int> fitting;
    for (int i = 0; i < static_cast<int>(list.size()); i++) {
        const Piece& p = list[i];
        bool fits = false;
        bool fills = false;
        for (int r = 0; r < board.size; r++) {
            for (int c = 0; c < board.size; c++) {
                if (!can_place(board, p, r, c)) {
                    continue;
                }
                fits = true;
                int region = label[(r + p.cells[0].row) * board.size + c + p.cells[0].col];
                bool same = true;
                for (const Cell& cell : p.cells) {
                    same = same && label[(r + cell.row) * board.size + c + cell.col] == region;
                }
                fills = fills || (same && sizes[region] == static_cast<int>(p.cells.size()));
            }
        }
        if (fills) {
            exact.push_back(i);
        }
        if (fits) {
            fitting.push_back(i);
        }
    }
    std::shuffle(exact.begin(), exact.end(), rng);
    std::shuffle(fitting.begin(), fitting.end(), rng);
    Tray tray{-1, -1, -1};
    int filled = 0;
    for (int index : exact) {
        if (filled < kTraySize) {
            tray[filled++] = index;
        }
    }
    for (int index : fitting) {
        bool used = std::find(tray.begin(), tray.end(), index) != tray.end();
        if (filled < kTraySize && !used) {
            tray[filled++] = index;
        }
    }
    while (filled < kTraySize && !fitting.empty()) {
        tray[filled++] = fitting[static_cast<size_t>(rng() % fitting.size())];
    }
    return tray;
}

Tray deal_fitting(std::mt19937& rng, const Board& board, uint64_t allowed)
{
    const std::vector<Piece>& list = piece_list();
    std::vector<int> fitting;
    for (int i = 0; i < static_cast<int>(list.size()); i++) {
        if ((allowed >> i & 1) && fits_anywhere(board, list[i])) {
            fitting.push_back(i);
        }
    }
    if (fitting.empty()) {
        return deal_tray(rng, board, allowed);
    }
    Tray tray{};
    for (int& slot : tray) {
        slot = fitting[static_cast<size_t>(rng() % fitting.size())];
    }
    return tray;
}

Tray deal_tray(std::mt19937& rng, const Board& board, uint64_t allowed)
{
    std::vector<int> pool;
    for (int i = 0; i < static_cast<int>(piece_list().size()); i++) {
        if (allowed >> i & 1) {
            pool.push_back(i);
        }
    }
    std::uniform_int_distribution<int> pick(0, static_cast<int>(pool.size()) - 1);
    Tray tray{};
    for (int attempt = 0; attempt < kDealTries; attempt++) {
        for (int& slot : tray) {
            slot = pool[static_cast<size_t>(pick(rng))];
        }
        if (any_piece_fits(board, tray)) {
            return tray;
        }
    }
    return tray;
}
