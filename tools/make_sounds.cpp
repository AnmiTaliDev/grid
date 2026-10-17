// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "synth.hpp"
#include "wav.hpp"

namespace {

bool write_file(const std::string& dir, const std::string& name, const std::vector<float>& samples)
{
    std::vector<uint8_t> bytes = encode_wav(samples);
    std::string path = dir + "/" + name;
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        std::fprintf(stderr, "cannot write %s\n", path.c_str());
        return false;
    }
    bool ok = std::fwrite(bytes.data(), 1, bytes.size(), f) == bytes.size();
    std::fclose(f);
    return ok;
}

}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: make_sounds <output directory>\n");
        return 2;
    }
    std::string dir = argv[1];
    bool ok = true;
    for (int i = 0; i < static_cast<int>(Sound::Count); i++) {
        Sound s = static_cast<Sound>(i);
        ok = write_file(dir, sound_file_name(s), synth_sound(s)) && ok;
    }
    for (int lines = 1; lines <= kMaxClearLines; lines++) {
        for (int step = 0; step < kMaxClearSteps; step++) {
            ok = write_file(dir, clear_file_name(lines, step), synth_clear(lines, step + 1)) && ok;
        }
    }
    return ok ? 0 : 1;
}
