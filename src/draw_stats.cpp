// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "draw.hpp"
#include <vector>

#include "format.hpp"

namespace {

constexpr float kPanelX = 52;
constexpr float kPanelW = 436;
constexpr float kContentX = 88;
constexpr float kContentW = 364;
constexpr float kTitleLine = 30.8f;
constexpr float kRow = 40;

struct Row {
    std::string label;
    std::string value;
};

std::vector<Row> make_rows(const StatsView& v)
{
    std::vector<Row> rows;
    Lang lang = v.lang;
    rows.push_back({tr(lang, Str::GamesPlayed), format_number(v.stats.games)});
    rows.push_back({tr(lang, Str::AverageScore), format_number(stats_average(v.stats))});
    rows.push_back({tr(lang, Str::LongestStreak), format_number(v.stats.best_streak)});
    rows.push_back({tr(lang, Str::LinesCleared), format_number(v.stats.lines)});
    rows.push_back({tr(lang, Str::PiecesPlaced), format_number(v.stats.pieces)});
    rows.push_back({tr(lang, Str::TimePlayed), format_duration(v.stats.seconds, lang)});
    for (int size = 6; size <= 8; size++) {
        std::string name = std::string(tr(lang, Str::Best)) + " " + std::to_string(size) + "x" + std::to_string(size);
        rows.push_back({name, format_number(v.bests.get(size))});
    }
    return rows;
}

}

StatsLayout stats_layout()
{
    StatsLayout l;
    l.row_height = kRow;
    float rows_h = 9 * kRow;
    float h = 36 + kTitleLine + 20 + rows_h + 28 + 64 + 36;
    float top = 480 - h / 2;
    l.panel = {kPanelX, top, kPanelW, h};
    l.title_top = top + 36;
    l.rows_top = l.title_top + kTitleLine + 20;
    l.back = {kContentX, l.rows_top + rows_h + 28, kContentW, 64};
    return l;
}

ButtonSpec stats_back_button(const StatsLayout& l, Lang lang)
{
    return {l.back, 16, 4, palette::kNight, palette::kPanelPrimaryEdge, tr(lang, Str::Back), {24, 800}, palette::kBone};
}

void draw_stats(Gfx& g, const StatsView& v)
{
    draw_backdrop(g);
    StatsLayout l = stats_layout();
    draw_sprite(g, panel_sprite(g, l.panel.h), l.panel.x, l.panel.y);
    draw_text_in_line(g, tr(v.lang, Str::StatsTitle), {28, 700}, kContentX, l.title_top, kTitleLine, palette::kPanelTitle);
    std::vector<Row> rows = make_rows(v);
    TextStyle label{16, 500};
    TextStyle value{18, 700};
    float value_n = normal_line_height(g, value);
    for (size_t i = 0; i < rows.size(); i++) {
        float y = l.rows_top + static_cast<float>(i) * l.row_height;
        float base = baseline_for_line(g, value, y + (l.row_height - value_n) / 2, value_n);
        draw_text(g, rows[i].label, label, kContentX, base, palette::kPanelText);
        float w = text_width(g, rows[i].value, value);
        draw_text(g, rows[i].value, value, kContentX + kContentW - w, base, palette::kInk);
        if (i + 1 < rows.size()) {
            fill_rect(g, {kContentX, y + l.row_height - 1, kContentW, 1}, with_alpha(palette::kPanelText, 0.18f));
        }
    }
    draw_button(g, stats_back_button(l, v.lang), v.pressed == 0, false);
}
