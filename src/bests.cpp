// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "bests.hpp"

#include <sstream>

int Bests::get(int size) const
{
    return size >= 0 && size <= kBoardSize ? by_size[size] : 0;
}

void Bests::set(int size, int score)
{
    if (size >= 0 && size <= kBoardSize && score >= 0) {
        by_size[size] = score;
    }
}

std::string serialize_bests(const Bests& bests)
{
    std::ostringstream out;
    out << "bests";
    for (int size = 0; size <= kBoardSize; size++) {
        if (bests.by_size[size] > 0) {
            out << " " << size << " " << bests.by_size[size];
        }
    }
    out << "\n";
    return out.str();
}

Bests parse_bests(const std::string& text)
{
    Bests bests;
    std::istringstream in(text);
    std::string first;
    if (!(in >> first)) {
        return bests;
    }
    if (first != "bests") {
        std::istringstream legacy(first);
        int value = 0;
        if (legacy >> value && value > 0) {
            bests.set(kBoardSize, value);
        }
        return bests;
    }
    int size = 0;
    int score = 0;
    while (in >> size >> score) {
        if (size >= 2 && size <= kBoardSize && score > 0) {
            bests.set(size, score);
        }
    }
    return bests;
}
