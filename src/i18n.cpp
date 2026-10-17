// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "i18n.hpp"

#include <array>
#include <cstring>

#include "kv.hpp"

namespace {

struct Entry {
    const char* key;
    const char* en;
};

const Entry kTable[] = {
    {"play", "Play"},
    {"continue", "Continue"},
    {"settings", "Settings"},
    {"stats", "Stats"},
    {"quit", "Quit"},
    {"best", "Best"},
    {"previous_best", "Previous best"},
    {"new_best", "New best"},
    {"no_moves_left", "No moves left"},
    {"rescue", "Rescue"},
    {"left", "left"},
    {"again", "Again"},
    {"menu", "Menu"},
    {"paused", "Paused"},
    {"resume", "Resume"},
    {"back", "Back"},
    {"streak", "streak"},
    {"undo", "Undo"},
    {"hold", "Hold"},
    {"on", "on"},
    {"off", "off"},
    {"none", "none"},
    {"set_grid", "Grid size"},
    {"set_lift", "Piece lift"},
    {"set_fullscreen", "Fullscreen"},
    {"set_deal", "Deal"},
    {"set_pieces", "Pieces"},
    {"set_rescues", "Rescues"},
    {"set_undo", "Undo"},
    {"set_hold", "Hold"},
    {"set_sound", "Sound"},
    {"set_vibration", "Vibration"},
    {"set_language", "Language"},
    {"reset_bests", "Reset bests"},
    {"confirm_reset", "Confirm reset"},
    {"deal_random", "random"},
    {"deal_fitting", "fitting"},
    {"deal_dynamic", "dynamic"},
    {"pieces_title", "Pieces"},
    {"stats_title", "Statistics"},
    {"games_played", "Games played"},
    {"average_score", "Average score"},
    {"longest_streak", "Longest streak"},
    {"lines_cleared", "Lines cleared"},
    {"pieces_placed", "Pieces placed"},
    {"time_played", "Time played"},
    {"unit_hour", "h"},
    {"unit_minute", "m"},
    {"unit_second", "s"},
    {"how_to_play", "How to play"},
    {"next", "Next"},
    {"start", "Start"},
    {"skip", "Skip"},
    {"undo_help", "takes back the last move"},
    {"hold_help", "keeps a piece for later"},
    {"rescue_help", "gives new pieces when stuck"},
    {"lines_one", "line"},
    {"lines_few", "lines"},
    {"lines_many", "lines"},
    {"language_name", "English"},
    {"tutorial_1_1", "Drag a piece from the tray"},
    {"tutorial_1_2", "onto the board."},
    {"tutorial_1_3", "Pieces cannot be rotated."},
    {"tutorial_2_1", "Fill a whole row or column"},
    {"tutorial_2_2", "to clear it. Clear lines in a row"},
    {"tutorial_2_3", "to build a streak."},
    {"tutorial_3_1", "The game ends when no piece fits."},
    {"tutorial_3_2", "Undo, Hold and Rescue can help."},
    {"tutorial_3_3", "All of them are set in Settings."},
};

static_assert(sizeof(kTable) / sizeof(kTable[0]) == static_cast<size_t>(Str::Count),
              "every Str needs an English and a Russian text");

}

namespace {

std::array<std::array<std::string, static_cast<size_t>(Str::Count)>, kLangCount>& loaded()
{
    static std::array<std::array<std::string, static_cast<size_t>(Str::Count)>, kLangCount> tables;
    return tables;
}

}

const char* str_key(Str id)
{
    return kTable[static_cast<size_t>(id)].key;
}

void i18n_reset()
{
    for (auto& table : loaded()) {
        for (std::string& s : table) {
            s.clear();
        }
    }
}

void i18n_load(Lang lang, const std::string& text)
{
    Kv kv = parse_kv(text);
    auto& table = loaded()[static_cast<size_t>(lang)];
    for (size_t i = 0; i < static_cast<size_t>(Str::Count); i++) {
        auto it = kv.values.find(kTable[i].key);
        if (it != kv.values.end() && !it->second.empty()) {
            table[i] = it->second;
        }
    }
}

const char* tr(Lang lang, Str id)
{
    size_t i = static_cast<size_t>(id);
    const std::string& text = loaded()[static_cast<size_t>(lang)][i];
    return text.empty() ? kTable[i].en : text.c_str();
}

Lang lang_from_index(int index)
{
    return index == 1 ? Lang::Russian : Lang::English;
}

Lang detect_language(const char* preferred)
{
    if (preferred && std::strncmp(preferred, "ru", 2) == 0) {
        return Lang::Russian;
    }
    return Lang::English;
}

std::string language_name(Lang lang)
{
    return tr(lang, Str::LanguageName);
}

int plural_form_ru(int count)
{
    int n = count < 0 ? -count : count;
    int last = n % 10;
    int last_two = n % 100;
    if (last == 1 && last_two != 11) {
        return 0;
    }
    if (last >= 2 && last <= 4 && (last_two < 12 || last_two > 14)) {
        return 1;
    }
    return 2;
}

std::string lines_text(Lang lang, int count)
{
    Str word = Str::LinesMany;
    if (lang == Lang::English) {
        word = count == 1 ? Str::LinesOne : Str::LinesMany;
    } else {
        int form = plural_form_ru(count);
        word = form == 0 ? Str::LinesOne : (form == 1 ? Str::LinesFew : Str::LinesMany);
    }
    return std::to_string(count) + " " + tr(lang, word);
}
