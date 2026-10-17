// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "settings.hpp"

#include "i18n.hpp"
#include "piece.hpp"

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

bool fullscreen_supported()
{
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)
    return true;
#else
    return false;
#endif
}

int next_grid(int grid)
{
    return next_in(kGridSizes, grid);
}

int next_lift(int lift)
{
    return next_in(kHandLifts, lift);
}

int next_rescues(int rescues)
{
    return next_in(kRescueCounts, rescues);
}

int next_undos(int undos)
{
    return next_in(kUndoCounts, undos);
}

int next_volume(int volume)
{
    return next_in(kVolumes, volume);
}

int next_language(int lang)
{
    return lang == 1 ? 0 : 1;
}

int next_deal(int deal)
{
    return (deal + 1) % 3;
}

Settings load_settings()
{
    Settings s;
    s.shapes = all_shapes();
    {
        int count = 0;
        SDL_Locale** locales = SDL_GetPreferredLocales(&count);
        const char* language = locales && count > 0 ? locales[0]->language : nullptr;
        s.lang = detect_language(language) == Lang::Russian ? 1 : 0;
        SDL_free(locales);
    }
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
    int rescues = s.rescues;
    int deal = s.deal;
    unsigned shapes = s.shapes;
    int undos = s.undos;
    int hold = s.hold ? 1 : 0;
    int volume = s.volume;
    int vibration = s.vibration ? 1 : 0;
    int tutorial = 0;
    int lang = s.lang;
    std::sscanf(text.c_str(),
                "grid %d lift %d fullscreen %d rescues %d deal %d shapes %u undos %d hold %d volume %d vibration %d "
                "tutorial %d lang %d",
                &grid, &lift, &full, &rescues, &deal, &shapes, &undos, &hold, &volume, &vibration, &tutorial,
                &lang);
    s.grid = allowed(kGridSizes, grid) ? grid : s.grid;
    s.lift = allowed(kHandLifts, lift) ? lift : s.lift;
    s.fullscreen = full != 0;
    s.rescues = allowed(kRescueCounts, rescues) ? rescues : s.rescues;
    s.deal = deal >= 0 && deal <= 2 ? deal : 0;
    s.undos = allowed(kUndoCounts, undos) ? undos : s.undos;
    s.hold = hold != 0;
    s.volume = allowed(kVolumes, volume) ? volume : s.volume;
    s.vibration = vibration != 0;
    s.tutorial_seen = tutorial != 0;
    s.lang = lang == 1 ? 1 : 0;
    shapes &= all_shapes();
    s.shapes = shapes != 0 ? shapes : s.shapes;
    return s;
}

void save_settings(const Settings& settings)
{
    std::string path = settings_path();
    if (path.empty()) {
        return;
    }
    std::string text = "grid " + std::to_string(settings.grid) + " lift " + std::to_string(settings.lift) +
                       " fullscreen " + (settings.fullscreen ? "1" : "0") + " rescues " + std::to_string(settings.rescues) +
                       " deal " + std::to_string(settings.deal) + " shapes " + std::to_string(settings.shapes) +
                       " undos " + std::to_string(settings.undos) + " hold " + (settings.hold ? "1" : "0") +
                       " volume " + std::to_string(settings.volume) +
                       " vibration " + (settings.vibration ? "1" : "0") +
                       " tutorial " + (settings.tutorial_seen ? "1" : "0") +
                       " lang " + std::to_string(settings.lang) + "\n";
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save settings: %s", SDL_GetError());
    }
}
