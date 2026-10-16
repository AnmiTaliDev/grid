// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

enum Glaze {
    kPersimmon,
    kSaffron,
    kCeladon,
    kCobalt,
    kOrchid,
    kBone,
};

struct Cell {
    int row;
    int col;
};

struct Piece {
    std::string_view name;
    int shape;
    int rotation;
    int glaze;
    int rows;
    int cols;
    std::vector<Cell> cells;
};

const std::vector<Piece>& piece_list();
int find_piece(std::string_view name, int rotation);
int shape_count();
int first_piece_of_shape(int shape);
uint64_t pieces_for_shapes(uint32_t shapes);
uint32_t all_shapes();
