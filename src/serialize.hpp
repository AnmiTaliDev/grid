// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <string>

#include "game.hpp"

std::string serialize_game(const Game& game, int previous_best);
bool deserialize_game(const std::string& text, Game& game, int& previous_best);
