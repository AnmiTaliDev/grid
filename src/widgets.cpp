// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "widgets.hpp"

void draw_button(Gfx& g, const ButtonSpec& b, bool pressed, bool focused)
{
    RaisedSpec spec{b.box.w, b.box.h, b.radius, b.edge, b.face, b.edge_color, pressed, b.glint};
    draw_sprite(g, raised_sprite(g, spec), b.box.x, b.box.y);
    Rect text_box = b.box;
    if (pressed) {
        text_box.y += b.edge;
    }
    draw_text_centered(g, b.label, b.style, text_box, b.label_color);
    if (focused) {
        draw_sprite(g, focus_ring_sprite(g, b.box.w, b.box.h), b.box.x - 6, b.box.y - 6);
    }
}
