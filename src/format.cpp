// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "format.hpp"

#include <cstdio>

std::string format_number(int value)
{
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
        if (count > 0 && count % 3 == 0) {
            out.insert(0, "\xC2\xA0");
        }
        out.insert(out.begin(), digits[i]);
        count++;
    }
    return out;
}

std::string format_duration(int64_t seconds, Lang lang)
{
    if (seconds < 0) {
        seconds = 0;
    }
    std::string h = tr(lang, Str::UnitHour);
    std::string m = tr(lang, Str::UnitMinute);
    std::string s = tr(lang, Str::UnitSecond);
    bool ru = lang == Lang::Russian;
    std::string gap = ru ? " " : "";
    int64_t hours = seconds / 3600;
    int64_t minutes = seconds % 3600 / 60;
    int64_t secs = seconds % 60;
    char buf[64];
    if (hours > 0) {
        std::snprintf(buf, sizeof(buf), "%lld%s%s %02lld%s%s", static_cast<long long>(hours), gap.c_str(), h.c_str(),
                      static_cast<long long>(minutes), gap.c_str(), m.c_str());
        return buf;
    }
    if (minutes > 0) {
        std::snprintf(buf, sizeof(buf), "%lld%s%s %02lld%s%s", static_cast<long long>(minutes), gap.c_str(),
                      m.c_str(), static_cast<long long>(secs), gap.c_str(), s.c_str());
        return buf;
    }
    return std::to_string(secs) + gap + s;
}
