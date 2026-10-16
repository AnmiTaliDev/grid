// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "piece.hpp"

#include <algorithm>

namespace {

struct BaseShape {
    std::string_view name;
    int glaze;
    std::vector<Cell> cells;
};

std::vector<BaseShape> base_shapes()
{
    return {
        {"dot", kSaffron, {{3, 3}}},
        {"bar2", kPersimmon, {{3, 2}, {3, 3}}},
        {"bar3", kCeladon, {{3, 2}, {3, 3}, {3, 4}}},
        {"bar4", kCobalt, {{3, 1}, {3, 2}, {3, 3}, {3, 4}}},
        {"bar5", kOrchid, {{3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}}},
        {"square2", kSaffron, {{2, 2}, {2, 3}, {3, 2}, {3, 3}}},
        {"block3x3", kPersimmon,
         {{2, 2}, {2, 3}, {2, 4}, {3, 2}, {3, 3}, {3, 4}, {4, 2}, {4, 3}, {4, 4}}},
        {"corner3", kCeladon, {{2, 2}, {3, 2}, {3, 3}}},
        {"corner5", kCobalt, {{2, 2}, {3, 2}, {4, 2}, {4, 3}, {4, 4}}},
        {"l4", kPersimmon, {{2, 2}, {3, 2}, {4, 2}, {4, 3}}},
        {"zigzag", kOrchid, {{2, 2}, {2, 3}, {3, 3}, {3, 4}}},
        {"tee", kSaffron, {{2, 2}, {2, 3}, {2, 4}, {3, 3}}},
        {"plus", kCeladon, {{1, 2}, {2, 1}, {2, 2}, {2, 3}, {3, 2}}},
        {"cup", kCobalt, {{2, 1}, {2, 3}, {3, 1}, {3, 2}, {3, 3}}},
        {"diag2", kOrchid, {{2, 2}, {3, 3}}},
        {"diag3", kSaffron, {{1, 1}, {2, 2}, {3, 3}}},
        {"stairs", kPersimmon, {{1, 1}, {2, 1}, {2, 2}, {3, 2}, {3, 3}}},
        {"skew5", kOrchid, {{1, 1}, {2, 1}, {3, 1}, {3, 2}, {4, 2}}},
    };
}

bool cell_less(const Cell& a, const Cell& b)
{
    return a.row != b.row ? a.row < b.row : a.col < b.col;
}

bool cell_equal(const Cell& a, const Cell& b)
{
    return a.row == b.row && a.col == b.col;
}

std::vector<Cell> normalize(std::vector<Cell> cells)
{
    int min_row = cells[0].row;
    int min_col = cells[0].col;
    for (const Cell& c : cells) {
        min_row = std::min(min_row, c.row);
        min_col = std::min(min_col, c.col);
    }
    for (Cell& c : cells) {
        c.row -= min_row;
        c.col -= min_col;
    }
    std::sort(cells.begin(), cells.end(), cell_less);
    return cells;
}

std::vector<Cell> rotate(const std::vector<Cell>& cells)
{
    std::vector<Cell> out;
    for (const Cell& c : cells) {
        out.push_back({c.col, -c.row});
    }
    return normalize(out);
}

bool same_cells(const std::vector<Cell>& a, const std::vector<Cell>& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), cell_equal);
}

std::vector<Piece> build_list()
{
    std::vector<Piece> list;
    int shape_index = -1;
    for (const BaseShape& shape : base_shapes()) {
        shape_index++;
        std::vector<std::vector<Cell>> seen;
        std::vector<Cell> cells = normalize(shape.cells);
        for (int i = 0; i < 4; i++) {
            bool duplicate = false;
            for (const auto& s : seen) {
                duplicate = duplicate || same_cells(s, cells);
            }
            if (!duplicate) {
                seen.push_back(cells);
                int rows = 0;
                int cols = 0;
                for (const Cell& c : cells) {
                    rows = std::max(rows, c.row + 1);
                    cols = std::max(cols, c.col + 1);
                }
                list.push_back({shape.name, shape_index, static_cast<int>(seen.size()) - 1, shape.glaze, rows, cols, cells});
            }
            cells = rotate(cells);
        }
    }
    return list;
}

}

const std::vector<Piece>& piece_list()
{
    static const std::vector<Piece> list = build_list();
    return list;
}

int shape_count()
{
    return piece_list().back().shape + 1;
}

int first_piece_of_shape(int shape)
{
    const std::vector<Piece>& list = piece_list();
    for (int i = 0; i < static_cast<int>(list.size()); i++) {
        if (list[i].shape == shape) {
            return i;
        }
    }
    return -1;
}

uint32_t all_shapes()
{
    return (uint32_t{1} << shape_count()) - 1;
}

uint64_t pieces_for_shapes(uint32_t shapes)
{
    const std::vector<Piece>& list = piece_list();
    uint64_t mask = 0;
    for (int i = 0; i < static_cast<int>(list.size()); i++) {
        if (shapes >> list[i].shape & 1) {
            mask |= uint64_t{1} << i;
        }
    }
    return mask;
}

int find_piece(std::string_view name, int rotation)
{
    const std::vector<Piece>& list = piece_list();
    for (int i = 0; i < static_cast<int>(list.size()); i++) {
        if (list[i].name == name && list[i].rotation == rotation) {
            return i;
        }
    }
    return -1;
}
