// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "assets.hpp"

#include "i18n.hpp"

#include <SDL3/SDL.h>

std::string asset_path(const std::string& relative)
{
#ifdef SDL_PLATFORM_ANDROID
    return relative;
#else
    const char* base = SDL_GetBasePath();
    std::string root = base ? base : "";
    const std::string candidates[] = {root + "assets/", root + "../share/grid/assets/"};
    for (const std::string& candidate : candidates) {
        if (SDL_GetPathInfo((candidate + relative).c_str(), nullptr)) {
            return candidate + relative;
        }
    }
    return candidates[0] + relative;
#endif
}

void load_translations()
{
    const char* files[kLangCount] = {"lang/en.txt", "lang/ru.txt"};
    for (int i = 0; i < kLangCount; i++) {
        size_t size = 0;
        void* data = SDL_LoadFile(asset_path(files[i]).c_str(), &size);
        if (!data) {
            SDL_Log("Cannot load %s: %s", files[i], SDL_GetError());
            continue;
        }
        i18n_load(lang_from_index(i), std::string(static_cast<const char*>(data), size));
        SDL_free(data);
    }
}
