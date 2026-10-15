// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "settings.hpp"

#include <SDL3/SDL.h>

#include <cstdio>
#include <string>

namespace {

std::string settings_path()
{
    char* dir = SDL_GetPrefPath("dev.anmitali", "grid");
    if (!dir) {
        return "";
    }
    std::string path = std::string(dir) + "settings.txt";
    SDL_free(dir);
    return path;
}

template <size_t N>
int next_in(const int (&values)[N], int current)
{
    for (size_t i = 0; i < N; i++) {
        if (values[i] == current) {
            return values[(i + 1) % N];
        }
    }
    return values[0];
}

template <size_t N>
bool allowed(const int (&values)[N], int value)
{
    for (int v : values) {
        if (v == value) {
            return true;
        }
    }
    return false;
}

}

int next_grid(int grid)
{
    return next_in(kGridSizes, grid);
}

int next_lift(int lift)
{
    return next_in(kHandLifts, lift);
}

Settings load_settings()
{
    Settings s;
    std::string path = settings_path();
    if (path.empty()) {
        return s;
    }
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        return s;
    }
    std::string text(static_cast<const char*>(data), size);
    SDL_free(data);
    int grid = s.grid;
    int lift = s.lift;
    int full = 0;
    std::sscanf(text.c_str(), "grid %d lift %d fullscreen %d", &grid, &lift, &full);
    s.grid = allowed(kGridSizes, grid) ? grid : s.grid;
    s.lift = allowed(kHandLifts, lift) ? lift : s.lift;
    s.fullscreen = full != 0;
    return s;
}

void save_settings(const Settings& settings)
{
    std::string path = settings_path();
    if (path.empty()) {
        return;
    }
    std::string text = "grid " + std::to_string(settings.grid) + " lift " + std::to_string(settings.lift) +
                       " fullscreen " + (settings.fullscreen ? "1" : "0") + "\n";
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save settings: %s", SDL_GetError());
    }
}
