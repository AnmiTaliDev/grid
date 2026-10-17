// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cmath>

#include "draw.hpp"
#include "tutorial.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;
constexpr float kArtH = 190;
constexpr float kCaptionH = 96;
constexpr float kCell = 32;
constexpr float kCols = 5;
constexpr float kRows = 4;

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

struct MiniBoard {
    float x;
    float y;
};

MiniBoard mini_board(const Rect& art)
{
    return {art.x + art.w - kCols * kCell, art.y + (art.h - kRows * kCell) / 2};
}

void draw_mini_wells(Gfx& g, const MiniBoard& b)
{
    for (int r = 0; r < static_cast<int>(kRows); r++) {
        for (int c = 0; c < static_cast<int>(kCols); c++) {
            draw_sprite(g, well_sprite(g, 28, false), b.x + kCell * static_cast<float>(c),
                        b.y + kCell * static_cast<float>(r));
        }
    }
}

void draw_tray_box(Gfx& g, const Rect& art)
{
    draw_sprite(g, small_slot_sprite(g, 100, 100), art.x, art.y + (art.h - 100) / 2);
}

void draw_piece_tiles(Gfx& g, const Piece& piece, float x, float y, float pitch, float size, int glaze, float alpha)
{
    for (const Cell& c : piece.cells) {
        draw_sprite(g, tile_sprite(g, size, glaze, false), x + pitch * static_cast<float>(c.col),
                    y + pitch * static_cast<float>(c.row), alpha);
    }
}

void draw_hint_arrow(Gfx& g, const Rect& art)
{
    float cx = art.x + 100 + (art.w - 100 - kCols * kCell) / 2;
    float cy = art.y + art.h / 2;
    TextStyle style{26, 800};
    float w = text_width(g, ">>", style);
    draw_text(g, ">>", style, cx - w / 2, cy + 8, palette::kBoneEdge);
}

void draw_page_one(Gfx& g, const Rect& art, float seconds)
{
    float t = tutorial_loop(seconds);
    MiniBoard b = mini_board(art);
    draw_tray_box(g, art);
    draw_mini_wells(g, b);
    draw_hint_arrow(g, art);
    const Piece& piece = piece_list()[find_piece("corner3", 0)];
    float tray_x = art.x + (100 - 2 * 22 + 4) / 2.0f;
    float tray_y = art.y + (art.h - 100) / 2 + (100 - 2 * 22 + 4) / 2.0f;
    float board_x = b.x + kCell * 1;
    float board_y = b.y + kCell * 1;
    float move = tutorial_ease(t, 0.6f, 1.8f);
    if (t < 0.6f) {
        draw_piece_tiles(g, piece, tray_x, tray_y, 22, 18, piece.glaze, 1.0f);
    } else if (t < 2.4f) {
        float x = lerp(tray_x, board_x, move);
        float y = lerp(tray_y, board_y, move);
        float size = lerp(18, 28, move);
        float pitch = lerp(22, kCell, move);
        draw_piece_tiles(g, piece, x, y, pitch, size, piece.glaze, 1.0f);
    } else {
        draw_piece_tiles(g, piece, board_x, board_y, kCell, 28, piece.glaze, 1.0f);
    }
}

void draw_page_two(Gfx& g, const Rect& art, float seconds)
{
    float t = tutorial_loop(seconds);
    MiniBoard b = mini_board(art);
    draw_tray_box(g, art);
    draw_mini_wells(g, b);
    draw_hint_arrow(g, art);
    int row = static_cast<int>(kRows) - 1;
    int gap = 2;
    const int glazes[5] = {kSaffron, kCobalt, -1, kPersimmon, kCeladon};
    float clear = tutorial_ease(t, 2.0f, 2.3f);
    for (int c = 0; c < static_cast<int>(kCols); c++) {
        if (c == gap) {
            continue;
        }
        float x = b.x + kCell * static_cast<float>(c);
        float y = b.y + kCell * static_cast<float>(row);
        draw_sprite(g, tile_sprite(g, 28, glazes[c], false), x, y, 1.0f - clear);
        draw_sprite(g, tile_sprite(g, 28, kBone, false), x, y, clear);
    }
    float tray_x = art.x + (100 - 28) / 2.0f;
    float tray_y = art.y + (art.h - 100) / 2 + (100 - 28) / 2.0f;
    float board_x = b.x + kCell * static_cast<float>(gap);
    float board_y = b.y + kCell * static_cast<float>(row);
    float move = tutorial_ease(t, 0.5f, 1.6f);
    float x = t < 0.5f ? tray_x : lerp(tray_x, board_x, move);
    float y = t < 0.5f ? tray_y : lerp(tray_y, board_y, move);
    int glaze = clear > 0.5f ? kBone : kSaffron;
    draw_sprite(g, tile_sprite(g, 28, glaze, false), x, y, 1.0f);
}

void draw_page_three(Gfx& g, const Rect& art, float seconds)
{
    float t = tutorial_loop(seconds);
    struct Item {
        const char* name;
        const char* text;
        int glaze;
    };
    const Item items[3] = {
        {"Undo", "takes back the last move", kCobalt},
        {"Hold", "keeps a piece for later", kSaffron},
        {"Rescue", "gives new pieces when stuck", kCeladon},
    };
    TextStyle name{20, 800};
    TextStyle text{16, 500};
    float n = normal_line_height(g, name);
    float row_h = 52;
    for (int i = 0; i < 3; i++) {
        float y = art.y + 10 + row_h * static_cast<float>(i);
        float pulse = tutorial_ease(t, 0.5f + 0.7f * static_cast<float>(i), 0.8f + 0.7f * static_cast<float>(i));
        draw_sprite(g, tile_sprite(g, 28, items[i].glaze, false), art.x, y + 4, 0.5f + 0.5f * pulse);
        float base = baseline_for_line(g, name, y, n);
        draw_text(g, items[i].name, name, art.x + 46, base, palette::kInk);
        float w = text_width(g, items[i].name, name);
        draw_text(g, items[i].text, text, art.x + 46 + w + 10, base, palette::kPanelText);
    }
}

}

TutorialLayout tutorial_layout()
{
    TutorialLayout l;
    float h = 36 + kTitleLine + 20 + kArtH + 16 + kCaptionH + 24 + 64 + 14 + 56 + 36;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    l.art = {kContentX, l.title_top + kTitleLine + 20, kContentW, kArtH};
    l.caption_top = l.art.y + kArtH + 16;
    l.next = {kContentX, l.caption_top + kCaptionH + 24, kContentW, 64};
    l.skip = {kContentX, l.next.y + 64 + 14, kContentW, 56};
    return l;
}

ButtonSpec tutorial_next_button(const TutorialLayout& l, bool last)
{
    return {l.next, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, last ? "Start" : "Next", {24, 800},
            palette::kBone};
}

ButtonSpec tutorial_skip_button(const TutorialLayout& l)
{
    return {l.skip, 16, 4, palette::kPanelSecondary, palette::kBoneEdge, "Skip", {18, 600}, palette::kInk};
}

void draw_tutorial(Gfx& g, const TutorialView& v)
{
    draw_backdrop(g);
    TutorialLayout l = tutorial_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, "How to play", {28, 700}, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    int page = std::clamp(v.page, 0, kTutorialPages - 1);
    float dots_w = static_cast<float>(kTutorialPages) * 14.0f;
    for (int i = 0; i < kTutorialPages; i++) {
        Color c = i == page ? palette::kPanelTitle : palette::kBoneEdge;
        fill_rect(g, {kContentX + kContentW - dots_w + 14.0f * static_cast<float>(i), l.title_top + 12, 8, 8}, c);
    }
    if (page == 0) {
        draw_page_one(g, l.art, v.seconds);
    } else if (page == 1) {
        draw_page_two(g, l.art, v.seconds);
    } else {
        draw_page_three(g, l.art, v.seconds);
    }
    TextStyle caption{18, 600};
    float n = normal_line_height(g, caption);
    const std::vector<std::string>& lines = tutorial_caption(page);
    for (size_t i = 0; i < lines.size(); i++) {
        draw_text_in_line(g, lines[i], caption, kContentX, l.caption_top + static_cast<float>(i) * (n + 4), n,
                          palette::kInk);
    }
    bool last = page == kTutorialPages - 1;
    draw_button(g, tutorial_next_button(l, last), v.pressed == 0, false);
    if (!last) {
        draw_button(g, tutorial_skip_button(l), v.pressed == 1, false);
    }
}
