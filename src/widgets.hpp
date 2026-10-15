// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <string>

#include "gfx.hpp"
#include "sprites.hpp"

struct ButtonSpec {
    Rect box;
    float radius;
    float edge;
    Color face;
    Color edge_color;
    std::string label;
    TextStyle style;
    Color label_color;
    bool glint = false;
};

void draw_button(Gfx& g, const ButtonSpec& b, bool pressed, bool focused);
