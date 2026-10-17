// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cmath>

#include "draw.hpp"
#include "drop.hpp"
#include "format.hpp"
#include "layout.hpp"

namespace {

float cell_x(int size, int col)
{
    return board_x(size) + kCellPitch * static_cast<float>(col);
}

float cell_y(int row)
{
    return kBoardY + kCellPitch * static_cast<float>(row);
}

void draw_bone_cell(Gfx& g, float x, float y, float bone_ms)
{
    const Sprite& bone = bone_tile_sprite(g);
    if (bone_ms <= kBoneHoldMs) {
        draw_sprite(g, bone, x, y);
        return;
    }
    float p = std::min(1.0f, (bone_ms - kBoneHoldMs) / kBoneFadeMs);
    if (p >= 1.0f) {
        return;
    }
    draw_sprite(g, well_sprite(g, 54, false), x, y);
    draw_sprite_scaled(g, bone, x, y, x + 27, y + 27, 1.0f - 0.7f * p, 1.0f - p);
}

void draw_cells(Gfx& g, const GameView& v, const Piece* hand, Drop drop)
{
    uint64_t ghost = 0;
    if (hand && drop.valid) {
        for (const Cell& c : hand->cells) {
            ghost |= cell_bit(drop.row + c.row, drop.col + c.col);
        }
    }
    for (int r = 0; r < v.board.size; r++) {
        for (int c = 0; c < v.board.size; c++) {
            float x = cell_x(v.board.size, c);
            float y = cell_y(r);
            uint64_t bit = cell_bit(r, c);
            int value = v.board.cell[r][c];
            if (v.bone_mask & bit) {
                draw_bone_cell(g, x, y, v.bone_ms);
            } else if (ghost & bit) {
                draw_sprite(g, ghost_sprite(g, hand->glaze), x, y);
            } else if (value == 0) {
                draw_sprite(g, well_sprite(g, 54, false), x, y);
            } else {
                draw_sprite(g, tile_sprite(g, 54, value - 1, false), x, y);
            }
        }
    }
}

void draw_would_clear(Gfx& g, const Board& board, const Piece& piece, Drop drop)
{
    Board after = board;
    place(after, piece, drop.row, drop.col);
    Lines lines = full_lines(after);
    float span = bed_width(board.size) - 12;
    float left = bed_x(board.size) + 6;
    for (int r = 0; r < board.size; r++) {
        if (lines.rows >> r & 1) {
            draw_sprite(g, would_clear_sprite(g, span, 66), left, 222 + kCellPitch * static_cast<float>(r));
        }
    }
    for (int c = 0; c < board.size; c++) {
        if (lines.cols >> c & 1) {
            draw_sprite(g, would_clear_sprite(g, 66, span), left + kCellPitch * static_cast<float>(c), 222);
        }
    }
}

void draw_popup(Gfx& g, const GameView& v)
{
    float t = std::clamp(v.popup_ms / kPopupMs, 0.0f, 1.0f);
    float rise = -12.0f * t;
    float alpha = 1.0f - t;
    TextStyle big{104, 900, -2.08f};
    std::string gain = "+" + std::to_string(v.popup_gain);
    float w = text_width(g, gain, big);
    float baseline = baseline_for_line(g, big, 236 + rise, 104);
    draw_text(g, gain, big, 270 - w / 2, baseline + 6, palette::kSaffronEdge, alpha);
    draw_text(g, gain, big, 270 - w / 2, baseline, palette::kSaffronFace, alpha);
    TextStyle label{22, 600};
    std::string text = v.popup_lines == 1 ? "1 line" : std::to_string(v.popup_lines) + " lines";
    float lw = text_width(g, text, label);
    draw_text_in_line(g, text, label, 270 - lw / 2, 354 + rise, normal_line_height(g, label), palette::kBone,
                      alpha);
}

void draw_hand(Gfx& g, const Piece& piece, float px, float py)
{
    Point o = footprint_origin(piece, px, py);
    for (const Cell& c : piece.cells) {
        draw_sprite(g, hand_tile_sprite(g, piece.glaze), o.x + kCellPitch * static_cast<float>(c.col),
                    o.y + kCellPitch * static_cast<float>(c.row));
    }
}

void draw_returning(Gfx& g, const Piece& piece, const GameView& v)
{
    float t = std::clamp(v.return_t, 0.0f, 1.0f);
    bool from_hold = v.return_slot == kTraySize;
    float slot_cx = from_hold ? kHoldX + kHoldW / 2 : kTrayXs[v.return_slot] + kTraySlotSize / 2;
    float slot_cy = from_hold ? kBottomY + kBottomH / 2 : kTrayY + kTraySlotSize / 2;
    float cx = v.pointer_x + (slot_cx - v.pointer_x) * t;
    float cy = v.pointer_y + (slot_cy - v.pointer_y) * t;
    float pitch = kCellPitch + (34.0f - kCellPitch) * t;
    float scale = 1.0f + (30.0f / 54.0f - 1.0f) * t;
    for (const Cell& c : piece.cells) {
        float tx = cx + (static_cast<float>(c.col) + 0.5f - static_cast<float>(piece.cols) / 2) * pitch;
        float ty = cy + (static_cast<float>(c.row) + 0.5f - static_cast<float>(piece.rows) / 2) * pitch;
        draw_sprite_scaled(g, hand_tile_sprite(g, piece.glaze), tx - 27, ty - 27, tx, ty, scale, 1.0f);
    }
}

void draw_tray(Gfx& g, const GameView& v)
{
    for (int i = 0; i < kTraySize; i++) {
        float sx = kTrayXs[i];
        int index = v.tray[i];
        bool away = i == v.hand_slot || i == v.return_slot;
        if (index < 0 || away) {
            draw_sprite(g, empty_slot_sprite(g), sx, kTrayY);
            continue;
        }
        draw_sprite(g, tray_slot_sprite(g), sx, kTrayY);
        const Piece& piece = piece_list()[index];
        float w = 30.0f * static_cast<float>(piece.cols) + 4.0f * static_cast<float>(piece.cols - 1);
        float h = 30.0f * static_cast<float>(piece.rows) + 4.0f * static_cast<float>(piece.rows - 1);
        float ox = sx + (kTraySlotSize - w) / 2;
        float oy = kTrayY + (kTraySlotSize - h) / 2;
        for (const Cell& c : piece.cells) {
            draw_sprite(g, tile_sprite(g, 30, piece.glaze, false), ox + 34.0f * static_cast<float>(c.col),
                        oy + 34.0f * static_cast<float>(c.row));
        }
    }
}

void draw_bottom_bar(Gfx& g, const GameView& v)
{
    if (v.undo_enabled) {
        bool usable = v.undo_available && v.undos_left > 0;
        RaisedSpec spec{kUndoW, kBottomH - 4, 14, 3, palette::kButtonFace, palette::kBedEdge, v.undo_pressed && usable};
        float alpha = usable ? 1.0f : 0.4f;
        draw_sprite(g, raised_sprite(g, spec), kUndoX, kBottomY, alpha);
        std::string label = "Undo (" + std::to_string(v.undos_left) + ")";
        Rect box = {kUndoX, kBottomY + (v.undo_pressed && usable ? 3.0f : 0.0f), kUndoW, kBottomH - 4};
        draw_text_centered(g, label, {16, 600}, box, palette::kBone, alpha);
    }
    if (v.hold_enabled) {
        draw_sprite(g, small_slot_sprite(g, kHoldW, kBottomH), kHoldX, kBottomY);
        bool away = v.hand_slot == kTraySize || v.return_slot == kTraySize;
        if (v.hold < 0 || away) {
            draw_text_centered(g, "Hold", {15, 600}, {kHoldX, kBottomY, kHoldW, kBottomH}, palette::kDim,
                               away && v.hold >= 0 ? 0.0f : 1.0f);
        } else {
            const Piece& piece = piece_list()[v.hold];
            float w = 11.0f * static_cast<float>(piece.cols) - 1.0f;
            float h = 11.0f * static_cast<float>(piece.rows) - 1.0f;
            float ox = kHoldX + (kHoldW - w) / 2;
            float oy = kBottomY + (kBottomH - h) / 2;
            for (const Cell& c : piece.cells) {
                draw_sprite(g, tile_sprite(g, 10, piece.glaze, false), ox + 11.0f * static_cast<float>(c.col),
                            oy + 11.0f * static_cast<float>(c.row));
            }
        }
    }
}

void draw_best_row(Gfx& g, int best)
{
    TextStyle label{15, 500};
    TextStyle number{20, 700};
    std::string num = format_number(best);
    float wl = text_width(g, "Best", label);
    float wn = text_width(g, num, number);
    float left = 512 - (wl + 10 + wn);
    float baseline = baseline_for_line(g, number, 38, normal_line_height(g, number));
    draw_text(g, "Best", label, left, baseline, palette::kDim);
    draw_text(g, num, number, left + wl + 10, baseline, palette::kBone);
}

void draw_streak_chip(Gfx& g, int streak)
{
    RaisedSpec spec{76, 76, 18, 4, palette::kSaffronFace, palette::kSaffronEdge};
    draw_sprite(g, raised_sprite(g, spec), 436, 104);
    draw_text_centered_x(g, "\xC3\x97" + std::to_string(streak), {32, 800}, 474, 118.2f, 32, palette::kInk);
    draw_text_centered_x(g, "streak", {13, 600}, 474, 150.2f, 15.6f, palette::kInk);
}

}

void draw_header_score(Gfx& g, int score, float alpha)
{
    TextStyle style{84, 800, -1.68f};
    draw_text_in_line(g, format_number(score), style, 28, 98, 84, palette::kBone, alpha);
}

void draw_game(Gfx& g, const GameView& v)
{
    fill_rect(g, {0, 0, kScreenW, kScreenH}, palette::kNight);
    draw_sprite(g, pause_button_sprite(g, v.pause_pressed), 28, 28);
    draw_best_row(g, v.best);
    draw_header_score(g, v.score, 1.0f);
    if (v.streak >= 2) {
        draw_streak_chip(g, v.streak);
    }
    draw_sprite(g, bed_sprite(g, false, v.board.size), bed_x(v.board.size), kBedY);
    const Piece* hand = nullptr;
    Drop drop;
    if (v.hand_slot >= 0 && view_piece(v, v.hand_slot) >= 0) {
        hand = &piece_list()[view_piece(v, v.hand_slot)];
        drop = find_drop(v.board, *hand, v.pointer_x, v.pointer_y);
    }
    draw_cells(g, v, hand, drop);
    if (hand && drop.valid) {
        draw_would_clear(g, v.board, *hand, drop);
    }
    if (v.popup) {
        draw_popup(g, v);
    }
    if (hand) {
        draw_hand(g, *hand, v.pointer_x, v.pointer_y);
    } else if (v.return_slot >= 0 && view_piece(v, v.return_slot) >= 0) {
        draw_returning(g, piece_list()[view_piece(v, v.return_slot)], v);
    }
    draw_tray(g, v);
    draw_bottom_bar(g, v);
}
