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

std::string format_duration(int64_t seconds)
{
    if (seconds < 0) {
        seconds = 0;
    }
    int64_t hours = seconds / 3600;
    int64_t minutes = seconds % 3600 / 60;
    int64_t secs = seconds % 60;
    if (hours > 0) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%lldh %02lldm", static_cast<long long>(hours), static_cast<long long>(minutes));
        return buf;
    }
    if (minutes > 0) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%lldm %02llds", static_cast<long long>(minutes), static_cast<long long>(secs));
        return buf;
    }
    return std::to_string(secs) + "s";
}
