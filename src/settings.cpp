// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "settings.hpp"

#include "i18n.hpp"
#include "piece.hpp"

#include <SDL3/SDL.h>

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
    int count = 0;
    SDL_Locale** locales = SDL_GetPreferredLocales(&count);
    const char* language = locales && count > 0 ? locales[0]->language : nullptr;
    s.lang = detect_language(language) == Lang::Russian ? 1 : 0;
    SDL_free(locales);
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
    return parse_settings(text, s);
}

void save_settings(const Settings& settings)
{
    std::string path = settings_path();
    if (path.empty()) {
        return;
    }
    std::string text = serialize_settings(settings);
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save settings: %s", SDL_GetError());
    }
}
