// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "storage.hpp"

#include <SDL3/SDL.h>

#include "serialize.hpp"

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

std::string game_path()
{
    char* dir = SDL_GetPrefPath("dev.anmitali", "grid");
    if (!dir) {
        return "";
    }
    std::string path = std::string(dir) + "game.txt";
    SDL_free(dir);
    return path;
}

}

bool load_saved_game(Game& game, int& previous_best)
{
    std::string path = game_path();
    if (path.empty()) {
        return false;
    }
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        return false;
    }
    std::string text(static_cast<const char*>(data), size);
    SDL_free(data);
    return deserialize_game(text, game, previous_best);
}

void save_current_game(const Game& game, int previous_best)
{
    std::string path = game_path();
    if (path.empty()) {
        return;
    }
    std::string text = serialize_game(game, previous_best);
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save game: %s", SDL_GetError());
    }
}

void delete_saved_game()
{
    std::string path = game_path();
    if (!path.empty()) {
        SDL_RemovePath(path.c_str());
    }
}

Bests load_bests()
{
    std::string path = best_path();
    if (path.empty()) {
        return Bests{};
    }
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        return Bests{};
    }
    std::string text(static_cast<const char*>(data), size);
    SDL_free(data);
    return parse_bests(text);
}

void save_bests(const Bests& bests)
{
    std::string path = best_path();
    if (path.empty()) {
        return;
    }
    std::string text = serialize_bests(bests);
    if (!SDL_SaveFile(path.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save best score: %s", SDL_GetError());
    }
}
