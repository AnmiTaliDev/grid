// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct Kv {
    std::map<std::string, std::string> values;

    bool has(const std::string& key) const;
    std::string get(const std::string& key, const std::string& fallback = "") const;
    int64_t get_int(const std::string& key, int64_t fallback, int64_t min, int64_t max) const;
    void set(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int64_t value);
};

Kv parse_kv(const std::string& text);
Kv parse_word_pairs(const std::string& text, bool skip_first_word);
std::string write_kv(const std::vector<std::pair<std::string, std::string>>& entries);
std::vector<std::string> split_words(const std::string& text);
bool parse_int(const std::string& text, int64_t& out);
