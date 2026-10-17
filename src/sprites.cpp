// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "sprites.hpp"

#include "layout.hpp"

namespace {

constexpr int kKindTile = 1;
constexpr int kKindBoneTile = 2;
constexpr int kKindHandTile = 3;
constexpr int kKindWell = 4;
constexpr int kKindGhost = 5;
constexpr int kKindBed = 6;
constexpr int kKindTraySlot = 7;
constexpr int kKindEmptySlot = 8;
constexpr int kKindRaised = 9;
constexpr int kKindPause = 10;
constexpr int kKindFocus = 11;
constexpr int kKindWouldClear = 12;
constexpr int kKindPanel = 13;
constexpr int kKindSmallSlot = 14;

uint32_t pack(Color c)
{
    return static_cast<uint32_t>(c.r) << 24 | static_cast<uint32_t>(c.g) << 16 | static_cast<uint32_t>(c.b) << 8 | c.a;
}

Color dc(Color c, bool dim)
{
    return dim ? dim_color(c) : c;
}

const Sprite* cached(Gfx& g, const SpriteKey& key)
{
    auto it = g.sprites.find(key);
    return it == g.sprites.end() ? nullptr : &it->second;
}

const Sprite& store(Gfx& g, const SpriteKey& key, const Canvas& canvas)
{
    return g.sprites.emplace(key, sprite_from_canvas(g, canvas)).first->second;
}

float tile_radius(float size)
{
    if (size >= 54.0f) {
        return 10.0f;
    }
    return size >= 28.0f ? 8.0f : size * 0.28f;
}

void paint_glint(Canvas& c, float x, float y, float s, Color color)
{
    fill_rr(c, {x + 0.17f * s, y + 0.15f * s, 0.30f * s, 0.09f * s}, 0.045f * s, color);
    fill_rr(c, {x + 0.52f * s, y + 0.15f * s, 0.09f * s, 0.09f * s}, 0.045f * s, color);
}

void paint_pause_icon(Canvas& c, float dy)
{
    fill_rr(c, {18, 17 + dy, 4, 14}, 1.5f, palette::kBone);
    fill_rr(c, {26, 17 + dy, 4, 14}, 1.5f, palette::kBone);
}

}

const Sprite& tile_sprite(Gfx& g, float size, int glaze, bool dim)
{
    SpriteKey key{kKindTile, size, 0, 0, 0, 0, static_cast<uint32_t>(glaze), 0, dim};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    float r = tile_radius(size);
    float e = size >= 28.0f ? 3.0f : 2.0f;
    Canvas c = make_canvas(size, size + e, 1);
    fill_rr(c, {0, e, size, size}, r, dc(palette::kGlazeEdge[glaze], dim));
    fill_rr(c, {0, 0, size, size}, r, dc(palette::kGlazeFace[glaze], dim));
    paint_glint(c, 0, 0, size, dc(with_alpha(rgb(0xFFFFFF), 0.55f), dim));
    return store(g, key, c);
}

const Sprite& bone_tile_sprite(Gfx& g)
{
    SpriteKey key{kKindBoneTile, 54};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(54, 57, 42);
    shadow_rr(c, {0, 0, 54, 54}, 10, 0, 24, 0, with_alpha(palette::kBone, 0.30f));
    fill_rr(c, {-5, -5, 64, 64}, 15, with_alpha(palette::kBone, 0.10f));
    fill_rr(c, {0, 3, 54, 54}, 10, palette::kBoneEdge);
    fill_rr(c, {0, 0, 54, 54}, 10, palette::kBone);
    paint_glint(c, 0, 0, 54, with_alpha(palette::kBoneEdge, 0.70f));
    return store(g, key, c);
}

const Sprite& hand_tile_sprite(Gfx& g, int glaze)
{
    SpriteKey key{kKindHandTile, 54, 0, 0, 0, 0, static_cast<uint32_t>(glaze)};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(54, 58, 64);
    shadow_rr(c, {0, 0, 54, 54}, 10, 22, 24, -4, with_alpha(rgb(0x000000), 0.55f));
    fill_rr(c, {0, 4, 54, 54}, 10, palette::kGlazeEdge[glaze]);
    fill_rr(c, {0, 0, 54, 54}, 10, palette::kGlazeFace[glaze]);
    paint_glint(c, 0, 0, 54, with_alpha(rgb(0xFFFFFF), 0.55f));
    return store(g, key, c);
}

const Sprite& well_sprite(Gfx& g, float size, bool dim)
{
    SpriteKey key{kKindWell, size, 0, 0, 0, 0, 0, 0, dim};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    float r = tile_radius(size);
    Canvas c = make_canvas(size, size + 1, 1);
    fill_rr(c, {0, 1, size, size}, r, dc(with_alpha(rgb(0xFFFFFF), 0.05f), dim));
    fill_rr(c, {0, 0, size, size}, r, dc(palette::kWell, dim));
    inset_band(c, {0, 0, size, size}, r, 2, dc(with_alpha(rgb(0x000000), 0.55f), dim));
    return store(g, key, c);
}

const Sprite& ghost_sprite(Gfx& g, int glaze)
{
    SpriteKey key{kKindGhost, 54, 0, 0, 0, 0, static_cast<uint32_t>(glaze)};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Color face = palette::kGlazeFace[glaze];
    Color bed = palette::kSlate;
    auto mix = [](uint8_t f, uint8_t b) { return static_cast<uint8_t>(f * 0.28f + b * 0.72f + 0.5f); };
    Color fill = {mix(face.r, bed.r), mix(face.g, bed.g), mix(face.b, bed.b), 255};
    Canvas c = make_canvas(54, 54, 1);
    fill_rr(c, {0, 0, 54, 54}, 10, fill);
    ring_rr(c, {0, 0, 54, 54}, 10, 2, face);
    return store(g, key, c);
}

const Sprite& bed_sprite(Gfx& g, bool dim, int size)
{
    SpriteKey key{kKindBed, static_cast<float>(size), 0, 0, 0, 0, 0, 0, dim};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    float w = bed_width(size);
    Canvas c = make_canvas(w, w + 8, 1);
    fill_rr(c, {0, 8, w, w}, 24, dc(palette::kBedEdge, dim));
    fill_rr(c, {0, 0, w, w}, 24, dc(palette::kSlate, dim));
    ring_rr(c, {0, 0, w, w}, 24, 1, dc(palette::kSlateRing, dim));
    return store(g, key, c);
}

const Sprite& tray_slot_sprite(Gfx& g)
{
    SpriteKey key{kKindTraySlot};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(148, 148, 1);
    fill_rr(c, {0, 0, 148, 148}, 22, palette::kTraySlot);
    inset_band(c, {0, 0, 148, 148}, 22, 2, with_alpha(rgb(0x000000), 0.40f));
    return store(g, key, c);
}

const Sprite& small_slot_sprite(Gfx& g, float w, float h)
{
    SpriteKey key{kKindSmallSlot, w, h};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(w, h, 1);
    fill_rr(c, {0, 0, w, h}, 16, palette::kTraySlot);
    inset_band(c, {0, 0, w, h}, 16, 2, with_alpha(rgb(0x000000), 0.40f));
    return store(g, key, c);
}

const Sprite& empty_slot_sprite(Gfx& g)
{
    SpriteKey key{kKindEmptySlot};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(148, 148, 1);
    paint(c, mask_dashed_rr(c, {0, 0, 148, 148}, 22, 2, 6, 6), palette::kSlateRing);
    return store(g, key, c);
}

const Sprite& raised_sprite(Gfx& g, const RaisedSpec& spec)
{
    SpriteKey key{kKindRaised, spec.w, spec.h, spec.radius, spec.edge, 0, pack(spec.face), pack(spec.edge_color),
                  (spec.pressed ? 1 : 0) | (spec.glint ? 2 : 0)};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(spec.w, spec.h + spec.edge, 1);
    float dy = spec.pressed ? spec.edge : 0.0f;
    if (!spec.pressed) {
        fill_rr(c, {0, spec.edge, spec.w, spec.h}, spec.radius, spec.edge_color);
    }
    fill_rr(c, {0, dy, spec.w, spec.h}, spec.radius, spec.face);
    if (spec.glint) {
        Color gc = with_alpha(palette::kBoneEdge, 0.70f);
        fill_rr(c, {14, 9 + dy, 26, 6}, 3, gc);
        fill_rr(c, {44, 9 + dy, 6, 6}, 3, gc);
    }
    return store(g, key, c);
}

const Sprite& pause_button_sprite(Gfx& g, bool pressed)
{
    SpriteKey key{kKindPause, 0, 0, 0, 0, 0, 0, 0, pressed};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(48, 51, 1);
    float dy = pressed ? 3.0f : 0.0f;
    if (!pressed) {
        fill_rr(c, {0, 3, 48, 48}, 14, palette::kBedEdge);
    }
    fill_rr(c, {0, dy, 48, 48}, 14, palette::kSlate);
    paint_pause_icon(c, dy);
    return store(g, key, c);
}

const Sprite& focus_ring_sprite(Gfx& g, float w, float h)
{
    SpriteKey key{kKindFocus, w, h};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(w + 12, h + 12, 1);
    ring_rr(c, {0, 0, w + 12, h + 12}, 20, 3, palette::kFocus);
    Sprite s = sprite_from_canvas(g, c);
    return g.sprites.emplace(key, s).first->second;
}

const Sprite& would_clear_sprite(Gfx& g, float w, float h)
{
    SpriteKey key{kKindWouldClear, w, h};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(w, h, 1);
    ring_rr(c, {0, 0, w, h}, 16, 2, with_alpha(palette::kBone, 0.70f));
    return store(g, key, c);
}

const Sprite& panel_sprite(Gfx& g, float height)
{
    SpriteKey key{kKindPanel, height};
    if (const Sprite* s = cached(g, key)) {
        return *s;
    }
    Canvas c = make_canvas(436, height + 8, 140);
    shadow_rr(c, {0, 0, 436, height}, 28, 40, 60, 0, with_alpha(rgb(0x000000), 0.50f));
    fill_rr(c, {0, 8, 436, height}, 28, palette::kBoneEdge);
    fill_rr(c, {0, 0, 436, height}, 28, palette::kBone);
    return store(g, key, c);
}

bool warm_sprites_step(Gfx& g, int step)
{
    if (step < 25) {
        int glaze = step / 5;
        int kind = step % 5;
        if (kind == 0) {
            tile_sprite(g, 54, glaze, false);
        } else if (kind == 1) {
            tile_sprite(g, 30, glaze, false);
        } else if (kind == 2) {
            tile_sprite(g, 28, glaze, false);
        } else if (kind == 3) {
            ghost_sprite(g, glaze);
        } else {
            hand_tile_sprite(g, glaze);
        }
        return true;
    }
    int rest = step - 25;
    if (rest < 5) {
        tile_sprite(g, 54, rest, true);
        return true;
    }
    switch (rest - 5) {
    case 0:
        bone_tile_sprite(g);
        return true;
    case 1:
        well_sprite(g, 54, false);
        well_sprite(g, 54, true);
        return true;
    case 2:
        well_sprite(g, 28, false);
        return true;
    case 3:
        bed_sprite(g, false, 8);
        return true;
    case 4:
        bed_sprite(g, true, 8);
        return true;
    case 5:
        tray_slot_sprite(g);
        empty_slot_sprite(g);
        pause_button_sprite(g, false);
        return true;
    default:
        return false;
    }
}
