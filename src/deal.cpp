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
    for (int i = 0; i < base_piece_count(); i++) {
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

namespace {

struct Candidate {
    int piece;
    int score;
};

int score_cells(const Board& board, const std::vector<Cell>& cells, const std::vector<int>& label,
                const std::vector<int>& sizes)
{
    int n = board.size;
    Board after = board;
    for (const Cell& cell : cells) {
        after.cell[cell.row][cell.col] = 1;
    }
    int lines = line_count(full_lines(after));
    int region = label[cells[0].row * n + cells[0].col];
    bool same = true;
    int contact = 0;
    for (const Cell& cell : cells) {
        same = same && label[cell.row * n + cell.col] == region;
        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};
        for (int k = 0; k < 4; k++) {
            int nr = cell.row + dr[k];
            int nc = cell.col + dc[k];
            contact += (nr < 0 || nc < 0 || nr >= n || nc >= n || board.cell[nr][nc] != 0) ? 1 : 0;
        }
    }
    bool exact = same && sizes[region] == static_cast<int>(cells.size());
    return lines * 100 + (exact ? 60 : 0) + contact * 3 + static_cast<int>(cells.size());
}

int best_fit_score(const Board& board, const Piece& p, const std::vector<int>& label, const std::vector<int>& sizes)
{
    int best = 0;
    for (int r = 0; r < board.size; r++) {
        for (int c = 0; c < board.size; c++) {
            if (!can_place(board, p, r, c)) {
                continue;
            }
            std::vector<Cell> cells;
            for (const Cell& cell : p.cells) {
                cells.push_back({r + cell.row, c + cell.col});
            }
            best = std::max(best, score_cells(board, cells, label, sizes));
        }
    }
    return best;
}

bool same_as_base_piece(const std::vector<Cell>& cells)
{
    int min_row = cells[0].row;
    int min_col = cells[0].col;
    for (const Cell& c : cells) {
        min_row = std::min(min_row, c.row);
        min_col = std::min(min_col, c.col);
    }
    std::vector<Cell> shape;
    for (const Cell& c : cells) {
        shape.push_back({c.row - min_row, c.col - min_col});
    }
    std::sort(shape.begin(), shape.end(), [](const Cell& a, const Cell& b) {
        return a.row != b.row ? a.row < b.row : a.col < b.col;
    });
    const std::vector<Piece>& list = piece_list();
    for (int i = 0; i < base_piece_count(); i++) {
        const std::vector<Cell>& other = list[i].cells;
        if (other.size() == shape.size() &&
            std::equal(other.begin(), other.end(), shape.begin(),
                       [](const Cell& a, const Cell& b) { return a.row == b.row && a.col == b.col; })) {
            return true;
        }
    }
    return false;
}

std::vector<Cell> grow_shape(std::mt19937& rng, const Board& board, Cell seed, int target)
{
    int n = board.size;
    std::vector<Cell> cells{seed};
    std::vector<char> taken(static_cast<size_t>(n) * n, 0);
    taken[seed.row * n + seed.col] = 1;
    while (static_cast<int>(cells.size()) < target) {
        std::vector<Cell> frontier;
        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};
        for (const Cell& cell : cells) {
            for (int k = 0; k < 4; k++) {
                int nr = cell.row + dr[k];
                int nc = cell.col + dc[k];
                if (nr >= 0 && nc >= 0 && nr < n && nc < n && !taken[nr * n + nc] && board.cell[nr][nc] == 0) {
                    frontier.push_back({nr, nc});
                }
            }
        }
        if (frontier.empty()) {
            break;
        }
        Cell next = frontier[static_cast<size_t>(rng() % frontier.size())];
        taken[next.row * n + next.col] = 1;
        cells.push_back(next);
    }
    return cells;
}

bool fits_small_box(const std::vector<Cell>& cells)
{
    int min_row = cells[0].row;
    int max_row = cells[0].row;
    int min_col = cells[0].col;
    int max_col = cells[0].col;
    for (const Cell& c : cells) {
        min_row = std::min(min_row, c.row);
        max_row = std::max(max_row, c.row);
        min_col = std::min(min_col, c.col);
        max_col = std::max(max_col, c.col);
    }
    return max_row - min_row < 4 && max_col - min_col < 4;
}

int invent_piece(std::mt19937& rng, const Board& board, const std::vector<int>& label,
                 const std::vector<int>& sizes)
{
    std::vector<Cell> empty;
    for (int r = 0; r < board.size; r++) {
        for (int c = 0; c < board.size; c++) {
            if (board.cell[r][c] == 0) {
                empty.push_back({r, c});
            }
        }
    }
    struct Invented {
        std::vector<Cell> cells;
        int score;
    };
    std::vector<Invented> found;
    for (int attempt = 0; attempt < 60 && !empty.empty(); attempt++) {
        Cell seed = empty[static_cast<size_t>(rng() % empty.size())];
        int target = 2 + static_cast<int>(rng() % 5);
        std::vector<Cell> cells = grow_shape(rng, board, seed, target);
        if (cells.size() < 2 || !fits_small_box(cells) || same_as_base_piece(cells)) {
            continue;
        }
        found.push_back({cells, score_cells(board, cells, label, sizes)});
    }
    if (found.empty()) {
        return -1;
    }
    std::sort(found.begin(), found.end(), [](const Invented& a, const Invented& b) { return a.score > b.score; });
    if (found.size() > 4) {
        found.resize(4);
    }
    int total = 0;
    for (const Invented& f : found) {
        total += f.score;
    }
    int roll = static_cast<int>(rng() % static_cast<unsigned>(total));
    size_t pick = 0;
    for (size_t k = 0; k < found.size(); k++) {
        roll -= found[k].score;
        if (roll < 0) {
            pick = k;
            break;
        }
    }
    return register_custom_piece(found[pick].cells, static_cast<int>(rng() % 5));
}

}

Tray deal_dynamic(std::mt19937& rng, const Board& board, uint64_t allowed)
{
    int filled = 0;
    for (int r = 0; r < board.size; r++) {
        for (int c = 0; c < board.size; c++) {
            filled += board.cell[r][c] != 0 ? 1 : 0;
        }
    }
    float fill = static_cast<float>(filled) / static_cast<float>(board.size * board.size);
    float assist = std::clamp((fill - 0.35f) / 0.40f, 0.0f, 1.0f);
    Tray tray = deal_tray(rng, board, allowed);
    if (assist <= 0.0f) {
        return tray;
    }
    const std::vector<Piece>& list = piece_list();
    std::vector<int> sizes;
    std::vector<int> label = label_regions(board, sizes);
    std::vector<Candidate> pool;
    for (int i = 0; i < base_piece_count(); i++) {
        if (allowed >> i & 1) {
            int score = best_fit_score(board, list[i], label, sizes);
            if (score > 0) {
                pool.push_back({i, score});
            }
        }
    }
    std::sort(pool.begin(), pool.end(), [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
    if (pool.size() > 6) {
        pool.resize(6);
    }
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    for (int& slot : tray) {
        if (chance(rng) >= assist) {
            continue;
        }
        int invented = invent_piece(rng, board, label, sizes);
        if (invented >= 0) {
            slot = invented;
            continue;
        }
        if (pool.empty()) {
            continue;
        }
        int total = 0;
        for (const Candidate& cand : pool) {
            total += cand.score;
        }
        int roll = static_cast<int>(rng() % static_cast<unsigned>(total));
        for (size_t k = 0; k < pool.size(); k++) {
            roll -= pool[k].score;
            if (roll < 0) {
                slot = pool[k].piece;
                if (pool.size() > 1) {
                    pool.erase(pool.begin() + static_cast<long>(k));
                }
                break;
            }
        }
    }
    return tray;
}

Tray deal_fitting(std::mt19937& rng, const Board& board, uint64_t allowed)
{
    const std::vector<Piece>& list = piece_list();
    std::vector<int> fitting;
    for (int i = 0; i < base_piece_count(); i++) {
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
    for (int i = 0; i < base_piece_count(); i++) {
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
