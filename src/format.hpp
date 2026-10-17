// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <string>

#include "i18n.hpp"

std::string format_number(int value);
std::string format_duration(int64_t seconds, Lang lang = Lang::English);
