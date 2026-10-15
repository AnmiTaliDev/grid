// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "format.hpp"

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
