// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "app.hpp"

#include <algorithm>

#include "draw.hpp"
#include "drop.hpp"
#include "layout.hpp"
#include "settings.hpp"
#include "storage.hpp"
#include "titlebar.hpp"

namespace {

constexpr float kOverDelayMs = 400.0f;
constexpr Rect kPauseButton = {28, 28, 48, 48};

bool inside(Rect r, float x, float y)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

void apply_titlebar(App& app, Gfx& g)
{
    bool full = (SDL_GetWindowFlags(app.window) & SDL_WINDOW_FULLSCREEN) != 0;
    app.bar_h = titlebar_supported() && !full ? kTitleBarH : 0.0f;
    SDL_SetRenderLogicalPresentation(g.renderer, kScreenW, kScreenH + static_cast<int>(app.bar_h),
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void bar_click(App& app, float x, float y)
{
    int hit = titlebar_hit(x, y);
    if (hit == kBarClose) {
        app.quit = true;
    } else if (hit == kBarMinimize) {
        SDL_MinimizeWindow(app.window);
    }
}

void commit_best(App& app)
{
    if (app.game.score.score > app.best) {
        app.best = app.game.score.score;
        save_best(app.best);
    }
}

void start_game(App& app)
{
    game_start(app.game, static_cast<uint32_t>(SDL_GetTicksNS()), app.settings.grid);
    app.previous_best = app.best;
    app.view = GameView{};
    app.over_pending = false;
    app.screen = Screen::Play;
}

void toggle_fullscreen(App& app)
{
    bool full = (SDL_GetWindowFlags(app.window) & SDL_WINDOW_FULLSCREEN) != 0;
    app.settings.fullscreen = !full;
    SDL_SetWindowFullscreen(app.window, !full);
    save_settings(app.settings);
}

void activate_menu(App& app, int which)
{
    if (which == kMenuPlay) {
        start_game(app);
    } else if (which == kMenuSettings) {
        app.reset_armed = false;
        app.panel_pressed = -1;
        app.screen = Screen::Settings;
    } else {
        app.quit = true;
    }
}

void cancel_hand(App& app)
{
    app.view.hand_slot = -1;
    app.view.return_slot = -1;
}

void go_menu(App& app)
{
    commit_best(app);
    cancel_hand(app);
    app.menu = MenuView{};
    app.menu.best = app.best;
    app.screen = Screen::Menu;
}

void sync_view(App& app)
{
    app.view.board = app.game.board;
    app.view.tray = app.game.tray;
    app.view.score = app.game.score.score;
    app.view.streak = app.game.score.streak;
    app.view.best = std::max(app.best, app.game.score.score);
}

int menu_hit(float x, float y)
{
    for (int i = 0; i < 3; i++) {
        if (inside(menu_button(i).box, x, y)) {
            return i;
        }
    }
    return -1;
}

void play_down(App& app, float x, float y)
{
    if (inside(kPauseButton, x, y)) {
        app.view.pause_pressed = true;
        return;
    }
    if (app.game.over) {
        return;
    }
    for (int i = 0; i < kTraySize; i++) {
        Rect slot = {kTrayXs[i], kTrayY, kTraySlotSize, kTraySlotSize};
        if (app.game.tray[i] >= 0 && inside(slot, x, y)) {
            app.view.return_slot = -1;
            app.view.hand_slot = i;
            app.view.pointer_x = x;
            app.view.pointer_y = y - static_cast<float>(app.settings.lift);
            return;
        }
    }
}

void finish_move(App& app, const MoveResult& result)
{
    if (result.lines > 0) {
        app.view.bone_mask = result.cleared;
        app.view.bone_ms = 0;
        app.view.popup = true;
        app.view.popup_gain = result.gain;
        app.view.popup_lines = result.lines;
        app.view.popup_ms = 0;
    }
    if (app.game.over) {
        app.over_pending = true;
        app.over_ms = 0;
    }
}

void play_up(App& app, float x, float y)
{
    if (app.view.pause_pressed) {
        app.view.pause_pressed = false;
        if (inside(kPauseButton, x, y)) {
            cancel_hand(app);
            app.screen = Screen::Pause;
        }
        return;
    }
    int slot = app.view.hand_slot;
    if (slot < 0) {
        return;
    }
    app.view.pointer_x = x;
    app.view.pointer_y = y - static_cast<float>(app.settings.lift);
    const Piece& piece = piece_list()[app.game.tray[slot]];
    Drop drop = find_drop(app.game.board, piece, app.view.pointer_x, app.view.pointer_y);
    app.view.hand_slot = -1;
    if (drop.valid) {
        finish_move(app, game_place(app.game, slot, drop.row, drop.col));
    } else {
        app.view.return_slot = slot;
        app.view.return_t = 0;
    }
}

int over_hit(Gfx& g, float x, float y, bool chip)
{
    OverLayout l = over_layout(g, chip);
    if (inside(l.again, x, y)) {
        return 0;
    }
    return inside(l.menu, x, y) ? 1 : -1;
}

int pause_hit(float x, float y)
{
    PauseLayout l = pause_layout();
    if (inside(l.resume, x, y)) {
        return 0;
    }
    return inside(l.menu, x, y) ? 1 : -1;
}

int settings_hit(float x, float y)
{
    SettingsLayout l = settings_layout();
    for (int i = 0; i < 4; i++) {
        if (inside(l.option[i], x, y)) {
            return i;
        }
    }
    return inside(l.back, x, y) ? 4 : -1;
}

void settings_activate(App& app, int hit)
{
    bool was_armed = app.reset_armed;
    app.reset_armed = false;
    if (hit == 0) {
        app.settings.grid = next_grid(app.settings.grid);
    } else if (hit == 1) {
        app.settings.lift = next_lift(app.settings.lift);
    } else if (hit == 2) {
        toggle_fullscreen(app);
    } else if (hit == 3) {
        if (was_armed) {
            app.best = 0;
            app.game.score = ScoreState{};
            save_best(0);
        } else {
            app.reset_armed = true;
        }
    } else {
        go_menu(app);
    }
    save_settings(app.settings);
}

void pointer_down(App& app, Gfx& g, float x, float y)
{
    if (app.screen == Screen::Menu) {
        app.menu.focus = -1;
        app.menu.pressed = menu_hit(x, y);
    } else if (app.screen == Screen::Play) {
        play_down(app, x, y);
    } else if (app.screen == Screen::Pause) {
        app.panel_pressed = pause_hit(x, y);
    } else if (app.screen == Screen::Settings) {
        app.panel_pressed = settings_hit(x, y);
    } else {
        bool chip = app.game.score.score > app.previous_best;
        app.panel_pressed = over_hit(g, x, y, chip);
    }
}

void pointer_move(App& app, float x, float y)
{
    if (app.screen == Screen::Play && app.view.hand_slot >= 0) {
        app.view.pointer_x = x;
        app.view.pointer_y = y - static_cast<float>(app.settings.lift);
    }
}

void pointer_up(App& app, Gfx& g, float x, float y)
{
    if (app.screen == Screen::Menu) {
        int hit = menu_hit(x, y);
        int pressed = app.menu.pressed;
        app.menu.pressed = -1;
        if (hit >= 0 && hit == pressed) {
            activate_menu(app, hit);
        }
    } else if (app.screen == Screen::Play) {
        play_up(app, x, y);
    } else if (app.screen == Screen::Pause) {
        int hit = pause_hit(x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            if (hit == 0) {
                app.screen = Screen::Play;
            } else {
                go_menu(app);
            }
        }
    } else if (app.screen == Screen::Settings) {
        int hit = settings_hit(x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            settings_activate(app, hit);
        }
    } else {
        bool chip = app.game.score.score > app.previous_best;
        int hit = over_hit(g, x, y, chip);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            if (hit == 0) {
                start_game(app);
            } else {
                go_menu(app);
            }
        }
    }
}

void key_down(App& app, const SDL_KeyboardEvent& key)
{
    if (key.repeat) {
        return;
    }
    if (key.key == SDLK_ESCAPE) {
        if (app.screen == Screen::Play && !app.over_pending) {
            cancel_hand(app);
            app.view.pause_pressed = false;
            app.screen = Screen::Pause;
        } else if (app.screen == Screen::Pause) {
            app.screen = Screen::Play;
        } else if (app.screen == Screen::Over || app.screen == Screen::Settings) {
            go_menu(app);
        }
        return;
    }
    if (app.screen != Screen::Menu) {
        return;
    }
    if (key.key == SDLK_TAB) {
        int step = (key.mod & SDL_KMOD_SHIFT) ? 2 : 1;
        app.menu.focus = app.menu.focus < 0 ? (step == 1 ? 0 : 2) : (app.menu.focus + step) % 3;
    } else if (key.key == SDLK_RETURN || key.key == SDLK_KP_ENTER) {
        activate_menu(app, app.menu.focus < 0 ? kMenuPlay : app.menu.focus);
    }
}

}

void app_init(App& app, Gfx& g, SDL_Window* window)
{
    app.window = window;
    if (titlebar_supported()) {
        SDL_SetWindowHitTest(window, titlebar_hit_test, nullptr);
    }
    apply_titlebar(app, g);
    app.best = load_best();
    app.settings = load_settings();
    if (app.settings.fullscreen) {
        SDL_SetWindowFullscreen(window, true);
    }
    app.menu.best = app.best;
}

void app_event(App& app, Gfx& g, SDL_Event& event)
{
    SDL_ConvertEventToRenderCoordinates(g.renderer, &event);
    switch (event.type) {
    case SDL_EVENT_QUIT:
        app.quit = true;
        break;
    case SDL_EVENT_KEY_DOWN:
        key_down(app, event.key);
        break;
    case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
    case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        app.settings.fullscreen = event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN;
        apply_titlebar(app, g);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (event.button.y < app.bar_h) {
                bar_click(app, event.button.x, event.button.y);
            } else {
                pointer_down(app, g, event.button.x, event.button.y - app.bar_h);
            }
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        pointer_move(app, event.motion.x, event.motion.y - app.bar_h);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_LEFT) {
            pointer_up(app, g, event.button.x, event.button.y - app.bar_h);
        }
        break;
    case SDL_EVENT_FINGER_DOWN:
        if (!app.finger_down) {
            app.finger_down = true;
            app.finger = event.tfinger.fingerID;
            pointer_down(app, g, event.tfinger.x, event.tfinger.y - app.bar_h);
        }
        break;
    case SDL_EVENT_FINGER_MOTION:
        if (app.finger_down && app.finger == event.tfinger.fingerID) {
            pointer_move(app, event.tfinger.x, event.tfinger.y - app.bar_h);
        }
        break;
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED:
        if (app.finger_down && app.finger == event.tfinger.fingerID) {
            app.finger_down = false;
            pointer_up(app, g, event.tfinger.x, event.tfinger.y - app.bar_h);
        }
        break;
    default:
        break;
    }
}

void app_update(App& app, float dt_ms)
{
    if (app.screen != Screen::Play && app.screen != Screen::Pause) {
        return;
    }
    if (app.screen == Screen::Pause) {
        return;
    }
    GameView& v = app.view;
    if (v.bone_mask) {
        v.bone_ms += dt_ms;
        if (v.bone_ms > kBoneHoldMs + kBoneFadeMs) {
            v.bone_mask = 0;
        }
    }
    if (v.popup) {
        v.popup_ms += dt_ms;
        if (v.popup_ms > kPopupMs) {
            v.popup = false;
        }
    }
    if (v.return_slot >= 0) {
        v.return_t += dt_ms / kReturnMs;
        if (v.return_t >= 1.0f) {
            v.return_slot = -1;
        }
    }
    if (app.over_pending) {
        app.over_ms += dt_ms;
        if (app.over_ms >= kOverDelayMs) {
            app.over_pending = false;
            commit_best(app);
            app.panel_pressed = -1;
            app.screen = Screen::Over;
        }
    }
}

void app_draw(App& app, Gfx& g)
{
    sync_view(app);
    if (app.bar_h > 0) {
        draw_titlebar(g);
        SDL_Rect area = {0, static_cast<int>(app.bar_h), kScreenW, kScreenH};
        SDL_SetRenderViewport(g.renderer, &area);
    }
    if (app.screen == Screen::Menu) {
        draw_menu(g, app.menu);
    } else if (app.screen == Screen::Play) {
        draw_game(g, app.view);
    } else if (app.screen == Screen::Pause) {
        PauseView pv;
        pv.board = app.game.board;
        pv.score = app.game.score.score;
        pv.pressed = app.panel_pressed;
        draw_pause(g, pv);
    } else if (app.screen == Screen::Settings) {
        SettingsView sv;
        sv.grid = app.settings.grid;
        sv.lift = app.settings.lift;
        sv.fullscreen = app.settings.fullscreen;
        sv.reset_armed = app.reset_armed;
        sv.pressed = app.panel_pressed;
        draw_settings(g, sv);
    } else {
        OverView ov;
        ov.board = app.game.board;
        ov.score = app.game.score.score;
        ov.previous_best = app.previous_best;
        ov.new_best = app.game.score.score > app.previous_best;
        ov.pressed = app.panel_pressed;
        draw_game_over(g, ov);
    }
    SDL_SetRenderViewport(g.renderer, nullptr);
}

void app_shutdown(App& app)
{
    commit_best(app);
    save_settings(app.settings);
}
