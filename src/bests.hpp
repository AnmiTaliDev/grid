// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <array>
#include <string>

#include "board.hpp"

struct Bests {
    std::array<int, kBoardSize + 1> by_size{};

    int get(int size) const;
    void set(int size, int score);
};

std::string serialize_bests(const Bests& bests);
Bests parse_bests(const std::string& text);
