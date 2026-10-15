// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

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
    int rotation;
    int glaze;
    int rows;
    int cols;
    std::vector<Cell> cells;
};

const std::vector<Piece>& piece_list();
int find_piece(std::string_view name, int rotation);
