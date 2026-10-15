// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "draw.hpp"

#include "format.hpp"
#include "layout.hpp"

namespace {

struct WordCell {
    int glaze;
    int row;
    int col;
};

const WordCell kWordmark[] = {
    {kPersimmon, 0, 1}, {kPersimmon, 0, 2}, {kPersimmon, 1, 0}, {kPersimmon, 1, 3}, {kPersimmon, 2, 0},
    {kPersimmon, 3, 0}, {kPersimmon, 3, 2}, {kPersimmon, 3, 3}, {kPersimmon, 4, 1}, {kPersimmon, 4, 2},
    {kSaffron, 0, 5}, {kSaffron, 0, 6}, {kSaffron, 0, 7}, {kSaffron, 1, 5}, {kSaffron, 1, 8},
    {kSaffron, 2, 5}, {kSaffron, 2, 6}, {kSaffron, 2, 7}, {kSaffron, 3, 5}, {kSaffron, 3, 7},
    {kSaffron, 4, 5}, {kSaffron, 4, 8},
    {kCeladon, 0, 10}, {kCeladon, 1, 10}, {kCeladon, 2, 10}, {kCeladon, 3, 10}, {kCeladon, 4, 10},
    {kCobalt, 0, 12}, {kCobalt, 0, 13}, {kCobalt, 0, 14}, {kCobalt, 1, 12}, {kCobalt, 1, 15},
    {kCobalt, 2, 12}, {kCobalt, 2, 15}, {kCobalt, 3, 12}, {kCobalt, 3, 15}, {kCobalt, 4, 12},
    {kCobalt, 4, 13}, {kCobalt, 4, 14},
};

}

ButtonSpec menu_button(int which)
{
    if (which == kMenuPlay) {
        return {{80, 544, 380, 60}, 14, 4, palette::kBone, palette::kBoneEdge, "Play", {26, 800, -0.26f},
                palette::kInk, true};
    }
    if (which == kMenuFullscreen) {
        return {{80, 640, 188, 60}, 14, 4, palette::kButtonFace, palette::kBedEdge, "Fullscreen", {17, 600},
                palette::kBone};
    }
    return {{272, 640, 188, 60}, 14, 4, palette::kButtonFace, palette::kBedEdge, "Quit", {17, 600}, palette::kBone};
}

void draw_menu(Gfx& g, const MenuView& v)
{
    fill_rect(g, {0, 0, kScreenW, kScreenH}, palette::kNight);
    const Sprite& well = well_sprite(g, 28, false);
    for (int r = 0; r < 30; r++) {
        for (int c = 0; c < 16; c++) {
            draw_sprite(g, well, static_cast<float>(16 + 32 * c), static_cast<float>(32 * r));
        }
    }
    SDL_FRect full = {0, 0, kScreenW, kScreenH};
    SDL_RenderTexture(g.renderer, g.vignette, nullptr, &full);
    for (const WordCell& w : kWordmark) {
        draw_sprite(g, tile_sprite(g, 28, w.glaze, false), static_cast<float>(16 + 32 * w.col),
                    static_cast<float>(224 + 32 * w.row));
    }
    for (int i = 0; i < 3; i++) {
        draw_button(g, menu_button(i), v.pressed == i, v.focus == i);
    }
    TextStyle best{15, 500};
    std::string text = "Best\xC2\xA0\xC2\xA0" + format_number(v.best);
    draw_text_centered_x(g, text, best, 270, 768, normal_line_height(g, best), palette::kDim);
}
