// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "serialize.hpp"

#include <sstream>

namespace {

constexpr int kSaveVersion = 1;

void write_tray_entry(std::ostringstream& out, int index)
{
    if (index < 0) {
        out << " -";
    } else if (index < base_piece_count()) {
        out << " b " << index;
    } else {
        const Piece& p = piece_list()[index];
        out << " c " << p.glaze << " " << p.cells.size();
        for (const Cell& c : p.cells) {
            out << " " << c.row << " " << c.col;
        }
    }
}

bool read_tray_entry(std::istringstream& in, int& index)
{
    std::string kind;
    if (!(in >> kind)) {
        return false;
    }
    if (kind == "-") {
        index = -1;
        return true;
    }
    if (kind == "b") {
        int value = 0;
        if (!(in >> value) || value < 0 || value >= base_piece_count()) {
            return false;
        }
        index = value;
        return true;
    }
    if (kind != "c") {
        return false;
    }
    int glaze = 0;
    int count = 0;
    if (!(in >> glaze >> count) || glaze < 0 || glaze > 4 || count < 1 || count > 8) {
        return false;
    }
    std::vector<Cell> cells;
    for (int i = 0; i < count; i++) {
        Cell c{};
        if (!(in >> c.row >> c.col) || c.row < 0 || c.col < 0 || c.row > 7 || c.col > 7) {
            return false;
        }
        cells.push_back(c);
    }
    index = register_custom_piece(cells, glaze);
    return true;
}

}

std::string serialize_game(const Game& game, int previous_best)
{
    std::ostringstream out;
    out << "grid-save " << kSaveVersion << " size " << game.board.size << " score " << game.score.score
        << " streak " << game.score.streak << " rescues " << game.rescues_left << " deal " << game.deal_mode
        << " allowed " << game.allowed << " previous " << previous_best << " cells";
    for (int r = 0; r < game.board.size; r++) {
        for (int c = 0; c < game.board.size; c++) {
            out << " " << game.board.cell[r][c];
        }
    }
    out << " tray";
    for (int index : game.tray) {
        write_tray_entry(out, index);
    }
    out << "\n";
    return out.str();
}

bool deserialize_game(const std::string& text, Game& game, int& previous_best)
{
    std::istringstream in(text);
    std::string tag;
    int version = 0;
    int size = 0;
    int score = 0;
    int streak = 0;
    int rescues = 0;
    int deal = 0;
    uint64_t allowed = 0;
    int previous = 0;
    std::string key;
    in >> tag >> version;
    if (tag != "grid-save" || version != kSaveVersion) {
        return false;
    }
    in >> key >> size >> key >> score >> key >> streak >> key >> rescues >> key >> deal >> key >> allowed >> key >>
        previous >> key;
    if (!in || size < 2 || size > kBoardSize || score < 0 || streak < 0 || rescues < 0 || rescues > 99 ||
        deal < 0 || deal > 2 || previous < 0 || key != "cells") {
        return false;
    }
    Game loaded;
    loaded.board.size = size;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int value = 0;
            if (!(in >> value) || value < 0 || value > 5) {
                return false;
            }
            loaded.board.cell[r][c] = value;
        }
    }
    in >> key;
    if (key != "tray") {
        return false;
    }
    bool any = false;
    for (int& slot : loaded.tray) {
        if (!read_tray_entry(in, slot)) {
            return false;
        }
        any = any || slot >= 0;
    }
    if (!any || !any_piece_fits(loaded.board, loaded.tray)) {
        return false;
    }
    loaded.score.score = score;
    loaded.score.streak = streak;
    loaded.rescues_left = rescues;
    loaded.deal_mode = deal;
    loaded.allowed = allowed;
    loaded.over = false;
    loaded.rng = game.rng;
    game = loaded;
    previous_best = previous;
    return true;
}
