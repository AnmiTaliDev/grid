// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "synth.hpp"

struct Pulse {
    int ms;
    int amplitude;
};

Pulse pulse_for(Sound sound);
Pulse pulse_for_clear(int lines, int streak);
