// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "bests.hpp"

#include <cstdint>
#include <utility>
#include <vector>

#include "kv.hpp"

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
    std::vector<std::pair<std::string, std::string>> entries;
    for (int size = 0; size <= kBoardSize; size++) {
        if (bests.by_size[size] > 0) {
            entries.push_back({"best" + std::to_string(size), std::to_string(bests.by_size[size])});
        }
    }
    return write_kv(entries);
}

Bests parse_bests(const std::string& text)
{
    Bests bests;
    if (text.find('=') != std::string::npos) {
        Kv kv = parse_kv(text);
        for (int size = 2; size <= kBoardSize; size++) {
            int64_t score = kv.get_int("best" + std::to_string(size), 0, 1, 2000000000);
            if (score > 0) {
                bests.set(size, static_cast<int>(score));
            }
        }
        return bests;
    }
    std::vector<std::string> words = split_words(text);
    if (words.empty()) {
        return bests;
    }
    int64_t value = 0;
    if (words[0] != "bests") {
        if (parse_int(words[0], value) && value > 0 && value <= 2000000000) {
            bests.set(kBoardSize, static_cast<int>(value));
        }
        return bests;
    }
    for (size_t i = 1; i + 1 < words.size(); i += 2) {
        int64_t size = 0;
        int64_t score = 0;
        if (parse_int(words[i], size) && parse_int(words[i + 1], score) && size >= 2 && size <= kBoardSize &&
            score > 0 && score <= 2000000000) {
            bests.set(static_cast<int>(size), static_cast<int>(score));
        }
    }
    return bests;
}
