// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "kv.hpp"
#include "piece.hpp"
#include "settings.hpp"

namespace {

int pick(const Kv& kv, const std::string& key, int fallback, const int* values, size_t count)
{
    int64_t v = kv.get_int(key, fallback, -1000000, 1000000);
    for (size_t i = 0; i < count; i++) {
        if (values[i] == v) {
            return static_cast<int>(v);
        }
    }
    return fallback;
}

}

std::string serialize_settings(const Settings& s)
{
    return write_kv({
        {"grid", std::to_string(s.grid)},
        {"lift", std::to_string(s.lift)},
        {"fullscreen", s.fullscreen ? "1" : "0"},
        {"deal", std::to_string(s.deal)},
        {"shapes", std::to_string(s.shapes)},
        {"rescues", std::to_string(s.rescues)},
        {"undos", std::to_string(s.undos)},
        {"hold", s.hold ? "1" : "0"},
        {"volume", std::to_string(s.volume)},
        {"vibration", s.vibration ? "1" : "0"},
        {"tutorial", s.tutorial_seen ? "1" : "0"},
        {"lang", std::to_string(s.lang)},
    });
}

Settings parse_settings(const std::string& text, const Settings& defaults)
{
    Kv kv = text.find('=') != std::string::npos ? parse_kv(text) : parse_word_pairs(text, false);
    Settings s = defaults;
    s.grid = pick(kv, "grid", s.grid, kGridSizes, sizeof(kGridSizes) / sizeof(int));
    s.lift = pick(kv, "lift", s.lift, kHandLifts, sizeof(kHandLifts) / sizeof(int));
    s.fullscreen = kv.get_int("fullscreen", s.fullscreen ? 1 : 0, 0, 1) != 0;
    s.deal = static_cast<int>(kv.get_int("deal", s.deal, 0, 2));
    int64_t shapes = kv.get_int("shapes", s.shapes, 0, 0xFFFFFFFFll) & all_shapes();
    s.shapes = shapes != 0 ? static_cast<uint32_t>(shapes) : s.shapes;
    s.rescues = pick(kv, "rescues", s.rescues, kRescueCounts, sizeof(kRescueCounts) / sizeof(int));
    s.undos = pick(kv, "undos", s.undos, kUndoCounts, sizeof(kUndoCounts) / sizeof(int));
    s.hold = kv.get_int("hold", s.hold ? 1 : 0, 0, 1) != 0;
    s.volume = pick(kv, "volume", s.volume, kVolumes, sizeof(kVolumes) / sizeof(int));
    s.vibration = kv.get_int("vibration", s.vibration ? 1 : 0, 0, 1) != 0;
    s.tutorial_seen = kv.get_int("tutorial", s.tutorial_seen ? 1 : 0, 0, 1) != 0;
    s.lang = static_cast<int>(kv.get_int("lang", s.lang, 0, 1));
    return s;
}
