// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "wav.hpp"

#include <algorithm>
#include <cmath>

#include "synth.hpp"

namespace {

void put32(std::vector<uint8_t>& out, uint32_t v)
{
    for (int i = 0; i < 4; i++) {
        out.push_back(static_cast<uint8_t>(v >> (8 * i)));
    }
}

void put16(std::vector<uint8_t>& out, uint16_t v)
{
    out.push_back(static_cast<uint8_t>(v));
    out.push_back(static_cast<uint8_t>(v >> 8));
}

void put_tag(std::vector<uint8_t>& out, const char* tag)
{
    for (int i = 0; i < 4; i++) {
        out.push_back(static_cast<uint8_t>(tag[i]));
    }
}

}

std::vector<int16_t> quantize_samples(const std::vector<float>& samples)
{
    std::vector<int16_t> out;
    out.reserve(samples.size());
    for (float s : samples) {
        out.push_back(static_cast<int16_t>(std::lround(std::clamp(s, -1.0f, 1.0f) * 32767.0f)));
    }
    return out;
}

std::vector<uint8_t> encode_wav(const std::vector<float>& samples)
{
    std::vector<int16_t> pcm = quantize_samples(samples);
    uint32_t data_bytes = static_cast<uint32_t>(pcm.size() * 2);
    std::vector<uint8_t> out;
    put_tag(out, "RIFF");
    put32(out, 36 + data_bytes);
    put_tag(out, "WAVE");
    put_tag(out, "fmt ");
    put32(out, 16);
    put16(out, 1);
    put16(out, 1);
    put32(out, kSampleRate);
    put32(out, kSampleRate * 2);
    put16(out, 2);
    put16(out, 16);
    put_tag(out, "data");
    put32(out, data_bytes);
    for (int16_t s : pcm) {
        put16(out, static_cast<uint16_t>(s));
    }
    return out;
}
