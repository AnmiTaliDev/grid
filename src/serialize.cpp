// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "serialize.hpp"

#include <cerrno>
#include <cstdlib>

#include "kv.hpp"

namespace {

constexpr int kSaveVersion = 2;

bool parse_u64(const std::string& text, uint64_t& out)
{
    if (text.empty() || text[0] == '-') {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    unsigned long long v = std::strtoull(text.c_str(), &end, 10);
    if (errno != 0 || end == text.c_str() || *end != '\0') {
        return false;
    }
    out = v;
    return true;
}

std::string entry_text(int index)
{
    if (index < 0) {
        return "-";
    }
    if (index < base_piece_count()) {
        return "b " + std::to_string(index);
    }
    const Piece& p = piece_list()[index];
    std::string text = "c " + std::to_string(p.glaze) + " " + std::to_string(p.cells.size());
    for (const Cell& c : p.cells) {
        text += " " + std::to_string(c.row) + " " + std::to_string(c.col);
    }
    return text;
}

bool read_entry(const std::vector<std::string>& words, size_t& at, int& index)
{
    if (at >= words.size()) {
        return false;
    }
    const std::string& kind = words[at++];
    int64_t value = 0;
    if (kind == "-") {
        index = -1;
        return true;
    }
    if (kind == "b") {
        if (at >= words.size() || !parse_int(words[at++], value) || value < 0 || value >= base_piece_count()) {
            return false;
        }
        index = static_cast<int>(value);
        return true;
    }
    if (kind != "c") {
        return false;
    }
    int64_t glaze = 0;
    int64_t count = 0;
    if (at + 1 >= words.size() || !parse_int(words[at++], glaze) || !parse_int(words[at++], count) || glaze < 0 ||
        glaze > 4 || count < 1 || count > 8) {
        return false;
    }
    std::vector<Cell> cells;
    for (int64_t i = 0; i < count; i++) {
        int64_t row = 0;
        int64_t col = 0;
        if (at + 1 >= words.size() || !parse_int(words[at++], row) || !parse_int(words[at++], col) || row < 0 ||
            col < 0 || row > 7 || col > 7) {
            return false;
        }
        cells.push_back({static_cast<int>(row), static_cast<int>(col)});
    }
    index = register_custom_piece(cells, static_cast<int>(glaze));
    return true;
}

}

std::string serialize_game(const Game& game, int previous_best)
{
    std::string cells;
    for (int r = 0; r < game.board.size; r++) {
        for (int c = 0; c < game.board.size; c++) {
            cells += (cells.empty() ? "" : " ") + std::to_string(game.board.cell[r][c]);
        }
    }
    std::string tray;
    for (int index : game.tray) {
        tray += (tray.empty() ? "" : " ") + entry_text(index);
    }
    return write_kv({
        {"version", std::to_string(kSaveVersion)},
        {"size", std::to_string(game.board.size)},
        {"score", std::to_string(game.score.score)},
        {"streak", std::to_string(game.score.streak)},
        {"idle", std::to_string(game.score.idle)},
        {"rescues", std::to_string(game.rescues_left)},
        {"deal", std::to_string(game.deal_mode)},
        {"allowed", std::to_string(game.allowed)},
        {"previous", std::to_string(previous_best)},
        {"cells", cells},
        {"tray", tray},
        {"hold", entry_text(game.hold)},
    });
}

bool deserialize_game(const std::string& text, Game& game, int& previous_best)
{
    Kv kv = parse_kv(text);
    if (kv.get_int("version", 0, 0, 1000) != kSaveVersion) {
        return false;
    }
    int64_t size = kv.get_int("size", 0, 2, kBoardSize);
    int64_t score = kv.get_int("score", -1, 0, 2000000000);
    int64_t streak = kv.get_int("streak", -1, 0, 2000000000);
    int64_t rescues = kv.get_int("rescues", -1, 0, 99);
    int64_t deal = kv.get_int("deal", -1, 0, 2);
    int64_t previous = kv.get_int("previous", -1, 0, 2000000000);
    uint64_t allowed = 0;
    if (size == 0 || score < 0 || streak < 0 || rescues < 0 || deal < 0 || previous < 0 || !kv.has("allowed") ||
        !parse_u64(kv.get("allowed"), allowed)) {
        return false;
    }
    Game loaded;
    loaded.board.size = static_cast<int>(size);
    std::vector<std::string> cells = split_words(kv.get("cells"));
    if (cells.size() != static_cast<size_t>(size * size)) {
        return false;
    }
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int64_t value = 0;
            if (!parse_int(cells[static_cast<size_t>(r * size + c)], value) || value < 0 || value > 5) {
                return false;
            }
            loaded.board.cell[r][c] = static_cast<int>(value);
        }
    }
    std::vector<std::string> tray = split_words(kv.get("tray"));
    size_t at = 0;
    bool any = false;
    for (int& slot : loaded.tray) {
        if (!read_entry(tray, at, slot)) {
            return false;
        }
        any = any || slot >= 0;
    }
    if (at != tray.size()) {
        return false;
    }
    if (kv.has("hold")) {
        std::vector<std::string> hold = split_words(kv.get("hold"));
        size_t h = 0;
        if (!read_entry(hold, h, loaded.hold) || h != hold.size() || loaded.hold >= base_piece_count()) {
            return false;
        }
        any = any || loaded.hold >= 0;
    }
    if (!any || !game_has_move(loaded)) {
        return false;
    }
    loaded.score.score = static_cast<int>(score);
    loaded.score.streak = static_cast<int>(streak);
    loaded.score.idle = static_cast<int>(kv.get_int("idle", 0, 0, kStreakGraceMoves));
    loaded.rescues_left = static_cast<int>(rescues);
    loaded.deal_mode = static_cast<int>(deal);
    loaded.allowed = allowed;
    loaded.over = false;
    loaded.rng = game.rng;
    game = loaded;
    previous_best = static_cast<int>(previous);
    return true;
}
