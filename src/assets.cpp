// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "assets.hpp"

#include <SDL3/SDL.h>

std::string asset_path(const std::string& relative)
{
#ifdef SDL_PLATFORM_ANDROID
    return relative;
#else
    const char* base = SDL_GetBasePath();
    return std::string(base ? base : "") + "assets/" + relative;
#endif
}
