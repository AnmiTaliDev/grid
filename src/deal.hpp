// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <array>
#include <random>

#include "board.hpp"

constexpr int kTraySize = 3;
constexpr int kDealTries = 20;

using Tray = std::array<int, kTraySize>;

Tray deal_tray(std::mt19937& rng, const Board& board);
bool any_piece_fits(const Board& board, const Tray& tray);
