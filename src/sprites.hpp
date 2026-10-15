// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "gfx.hpp"

struct RaisedSpec {
    float w;
    float h;
    float radius;
    float edge;
    Color face;
    Color edge_color;
    bool pressed = false;
    bool glint = false;
};

const Sprite& tile_sprite(Gfx& g, float size, int glaze, bool dim);
const Sprite& bone_tile_sprite(Gfx& g);
const Sprite& hand_tile_sprite(Gfx& g, int glaze);
const Sprite& well_sprite(Gfx& g, float size, bool dim);
const Sprite& ghost_sprite(Gfx& g, int glaze);
const Sprite& bed_sprite(Gfx& g, bool dim);
const Sprite& tray_slot_sprite(Gfx& g);
const Sprite& empty_slot_sprite(Gfx& g);
const Sprite& raised_sprite(Gfx& g, const RaisedSpec& spec);
const Sprite& pause_button_sprite(Gfx& g, bool pressed);
const Sprite& focus_ring_sprite(Gfx& g, float w, float h);
const Sprite& would_clear_sprite(Gfx& g, float w, float h);
const Sprite& panel_sprite(Gfx& g, float height);

void warm_sprites(Gfx& g);
