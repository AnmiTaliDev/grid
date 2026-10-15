// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "storage.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <string>

namespace {

std::string best_path()
{
    char* dir = SDL_GetPrefPath("dev.anmitali", "grid");
    if (!dir) {
        return "";
    }
    std::string path = std::string(dir) + "best.txt";
    SDL_free(dir);
    return path;
}

}

int load_best()
{
    std::string path = best_path();
    if (path.empty()) {
        return 0;
    }
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        return 0;
    }
    std::string text(static_cast<const char*>(data), size);
    SDL_free(data);
    return std::max(0, std::atoi(text.c_str()));
}

void save_best(int best)
{
    std::string path = best_path();
    if (path.empty()) {
        return;
    }
    std::string text = std::to_string(best) + "\n";
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save best score: %s", SDL_GetError());
    }
}
