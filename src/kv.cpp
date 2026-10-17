// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "kv.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <sstream>

namespace {

std::string trim(const std::string& s)
{
    size_t begin = 0;
    size_t end = s.size();
    while (begin < end && (s[begin] == ' ' || s[begin] == '\t' || s[begin] == '\r' || s[begin] == '\n')) {
        begin++;
    }
    while (end > begin && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n')) {
        end--;
    }
    return s.substr(begin, end - begin);
}

}

bool Kv::has(const std::string& key) const
{
    return values.find(key) != values.end();
}

std::string Kv::get(const std::string& key, const std::string& fallback) const
{
    auto it = values.find(key);
    return it == values.end() ? fallback : it->second;
}

int64_t Kv::get_int(const std::string& key, int64_t fallback, int64_t min, int64_t max) const
{
    auto it = values.find(key);
    int64_t value = 0;
    if (it == values.end() || !parse_int(it->second, value)) {
        return fallback;
    }
    return value < min || value > max ? fallback : value;
}

void Kv::set(const std::string& key, const std::string& value)
{
    values[key] = value;
}

void Kv::set_int(const std::string& key, int64_t value)
{
    values[key] = std::to_string(value);
}

bool parse_int(const std::string& text, int64_t& out)
{
    std::string t = trim(text);
    if (t.empty()) {
        return false;
    }
    errno = 0;
    char* end = nullptr;
    long long v = std::strtoll(t.c_str(), &end, 10);
    if (errno != 0 || end == t.c_str() || *end != '\0') {
        return false;
    }
    out = v;
    return true;
}

Kv parse_kv(const std::string& text)
{
    Kv kv;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') {
            continue;
        }
        size_t eq = t.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim(t.substr(0, eq));
        if (key.empty()) {
            continue;
        }
        kv.values[key] = trim(t.substr(eq + 1));
    }
    return kv;
}

std::vector<std::string> split_words(const std::string& text)
{
    std::vector<std::string> words;
    std::istringstream in(text);
    std::string w;
    while (in >> w) {
        words.push_back(w);
    }
    return words;
}

Kv parse_word_pairs(const std::string& text, bool skip_first_word)
{
    Kv kv;
    std::vector<std::string> words = split_words(text);
    size_t i = skip_first_word ? 1 : 0;
    for (; i + 1 < words.size(); i += 2) {
        kv.values[words[i]] = words[i + 1];
    }
    return kv;
}

std::string write_kv(const std::vector<std::pair<std::string, std::string>>& entries)
{
    std::string out;
    for (const auto& [key, value] : entries) {
        out += key + " = " + value + "\n";
    }
    return out;
}
