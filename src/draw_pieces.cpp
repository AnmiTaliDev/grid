// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "draw.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;
constexpr float kCardW = 112;
constexpr float kCardH = 88;
constexpr float kCardGap = 14;
constexpr float kTile = 18;
constexpr float kPitch = 20;
constexpr int kColumns = 3;

}

PiecesLayout pieces_layout()
{
    PiecesLayout l;
    l.count = shape_count();
    int rows = (l.count + kColumns - 1) / kColumns;
    float grid_h = static_cast<float>(rows) * kCardH + static_cast<float>(rows - 1) * kCardGap;
    float h = 36 + kTitleLine + 24 + grid_h + 28 + 64 + 36;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    float y0 = l.title_top + kTitleLine + 24;
    for (int i = 0; i < l.count; i++) {
        float x = kContentX + static_cast<float>(i % kColumns) * (kCardW + kCardGap);
        float y = y0 + static_cast<float>(i / kColumns) * (kCardH + kCardGap);
        l.card[i] = {x, y, kCardW, kCardH};
    }
    l.back = {kContentX, y0 + grid_h + 28, kContentW, 64};
    return l;
}

ButtonSpec pieces_back_button(const PiecesLayout& l, Lang lang)
{
    return {l.back, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, tr(lang, Str::Back), {24, 800}, palette::kBone};
}

void draw_pieces(Gfx& g, const PiecesView& v)
{
    draw_backdrop(g);
    PiecesLayout l = pieces_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, tr(v.lang, Str::PiecesTitle), {28, 700}, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    for (int i = 0; i < l.count; i++) {
        bool on = v.shapes >> i & 1;
        const Rect& r = l.card[i];
        RaisedSpec card{r.w, r.h, 14, 4, palette::kPanelSecondary, palette::kBoneEdge, !on || v.pressed == i};
        draw_sprite(g, raised_sprite(g, card), r.x, r.y);
        const Piece& piece = piece_list()[first_piece_of_shape(i)];
        float w = kPitch * static_cast<float>(piece.cols) - (kPitch - kTile);
        float h = kPitch * static_cast<float>(piece.rows) - (kPitch - kTile);
        float shift = on && v.pressed != i ? 0.0f : 4.0f;
        float ox = r.x + (r.w - w) / 2;
        float oy = r.y + (r.h - h) / 2 + shift;
        for (const Cell& c : piece.cells) {
            draw_sprite(g, tile_sprite(g, kTile, piece.glaze, false), ox + kPitch * static_cast<float>(c.col),
                        oy + kPitch * static_cast<float>(c.row), on ? 1.0f : 0.3f);
        }
    }
    draw_button(g, pieces_back_button(l, v.lang), v.pressed == l.count, false);
}
