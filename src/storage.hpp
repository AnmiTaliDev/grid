// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "bests.hpp"
#include "game.hpp"

Bests load_bests();
void save_bests(const Bests& bests);
bool load_saved_game(Game& game, int& previous_best);
void save_current_game(const Game& game, int previous_best);
void delete_saved_game();
