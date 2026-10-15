// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "draw.hpp"
#include "format.hpp"
#include "layout.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;
const TextStyle kTitleStyle{28, 700};
const TextStyle kScoreStyle{96, 800, -1.92f};
const TextStyle kChipStyle{16, 700};
const TextStyle kLineStyle{16, 500};

void draw_dimmed_base(Gfx& g, const Board& board, int score)
{
    fill_rect(g, {0, 0, kScreenW, kScreenH}, palette::kNight);
    draw_sprite(g, pause_button_sprite(g, false), 28, 28, 0.35f);
    draw_header_score(g, score, 0.35f);
    draw_sprite(g, bed_sprite(g, true, board.size), bed_x(board.size), kBedY);
    for (int r = 0; r < board.size; r++) {
        for (int c = 0; c < board.size; c++) {
            float x = board_x(board.size) + kCellPitch * static_cast<float>(c);
            float y = kBoardY + kCellPitch * static_cast<float>(r);
            int value = board.cell[r][c];
            if (value == 0) {
                draw_sprite(g, well_sprite(g, 54, true), x, y);
            } else {
                draw_sprite(g, tile_sprite(g, 54, value - 1, true), x, y);
            }
        }
    }
}

ButtonSpec primary_button(Rect box, const char* label)
{
    return {box, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, label, {24, 800}, palette::kBone};
}

ButtonSpec secondary_button(Rect box, const char* label)
{
    return {box, 16, 4, palette::kPanelSecondary, palette::kBoneEdge, label, {18, 600}, palette::kInk};
}

}

OverLayout over_layout(Gfx& g, bool chip)
{
    float n = normal_line_height(g, kChipStyle);
    OverLayout l;
    l.title_top = 286;
    l.score_top = 286 + kTitleLine + 10;
    float score_bottom = l.score_top + 96;
    float cursor = score_bottom + 16;
    l.chip = {kContentX, cursor, 0, n + 16};
    if (chip) {
        l.chip.w = text_width(g, "New best", kChipStyle) + 28;
        cursor = cursor + n + 16 + 14;
    }
    l.line_top = cursor;
    float again_top = l.line_top + n + 28;
    l.again = {kContentX, again_top, kContentW, 64};
    l.menu = {kContentX, again_top + 64 + 14, kContentW, 56};
    l.panel = {kPanelX, 250, kPanelW, l.menu.y + 56 + 36 - 250};
    return l;
}

PauseLayout pause_layout()
{
    PauseLayout l;
    float h = 36 + kTitleLine + 28 + 64 + 14 + 56 + 36;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    l.resume = {kContentX, l.title_top + kTitleLine + 28, kContentW, 64};
    l.menu = {kContentX, l.resume.y + 64 + 14, kContentW, 56};
    return l;
}

ButtonSpec over_again_button(const OverLayout& l)
{
    return primary_button(l.again, "Again");
}

ButtonSpec over_menu_button(const OverLayout& l)
{
    return secondary_button(l.menu, "Menu");
}

ButtonSpec pause_resume_button(const PauseLayout& l)
{
    return primary_button(l.resume, "Resume");
}

ButtonSpec pause_menu_button(const PauseLayout& l)
{
    return secondary_button(l.menu, "Menu");
}

void draw_game_over(Gfx& g, const OverView& v)
{
    draw_dimmed_base(g, v.board, v.score);
    OverLayout l = over_layout(g, v.new_best);
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, "No moves left", kTitleStyle, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    draw_text_in_line(g, format_number(v.score), kScoreStyle, kContentX, l.score_top, 96, palette::kInk);
    float n = normal_line_height(g, kChipStyle);
    if (v.new_best) {
        RaisedSpec chip{l.chip.w, l.chip.h, 12, 3, palette::kPersimmonFace, palette::kPersimmonEdge};
        draw_sprite(g, raised_sprite(g, chip), l.chip.x, l.chip.y);
        draw_text_in_line(g, "New best", kChipStyle, l.chip.x + 14, l.chip.y + 8, n, palette::kInk);
    }
    std::string line = (v.new_best ? "Previous best " : "Best ") + format_number(v.previous_best);
    draw_text_in_line(g, line, kLineStyle, kContentX, l.line_top, normal_line_height(g, kLineStyle),
                      palette::kPanelText);
    draw_button(g, over_again_button(l), v.pressed == 0, false);
    draw_button(g, over_menu_button(l), v.pressed == 1, false);
}

void draw_pause(Gfx& g, const PauseView& v)
{
    draw_dimmed_base(g, v.board, v.score);
    PauseLayout l = pause_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, "Paused", kTitleStyle, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    draw_button(g, pause_resume_button(l), v.pressed == 0, false);
    draw_button(g, pause_menu_button(l), v.pressed == 1, false);
}

void warm_panels(Gfx& g)
{
    panel_sprite(g, over_layout(g, true).panel.h);
    panel_sprite(g, over_layout(g, false).panel.h);
    panel_sprite(g, pause_layout().panel.h);
}
