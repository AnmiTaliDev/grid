// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "app.hpp"

#include <algorithm>
#include <bit>

#include "draw.hpp"
#include "drop.hpp"
#include "haptics.hpp"
#include "layout.hpp"
#include "piece.hpp"
#include "settings.hpp"
#include "storage.hpp"
#include "tutorial.hpp"
#include "titlebar.hpp"

namespace {

constexpr float kOverDelayMs = 400.0f;
constexpr Rect kPauseButton = {28, 28, 48, 48};
constexpr Rect kUndoButton = {kUndoX, kBottomY, kUndoW, kBottomH};
constexpr Rect kHoldRect = {kHoldX, kBottomY, kHoldW, kBottomH};
constexpr float kHoldReach = 24.0f;

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

void sfx(App& app, Sound sound)
{
    audio_play(app.audio, sound);
    if (app.settings.vibration && sound != Sound::Click && sound != Sound::Pick) {
        haptics_pulse(pulse_for(sound));
    }
}

void sfx_clear(App& app, int lines, int streak)
{
    audio_play_clear(app.audio, lines, streak);
    if (app.settings.vibration) {
        haptics_pulse(pulse_for_clear(lines, streak));
    }
}

void persist_game(App& app)
{
    save_stats(app.stats);
    bool active = (app.screen == Screen::Play || app.screen == Screen::Pause) && !app.game.over;
    if (active) {
        save_current_game(app.game, app.previous_best);
        app.menu.has_save = true;
    }
}

void drop_saved_game(App& app)
{
    delete_saved_game();
    app.menu.has_save = false;
}

void commit_best(App& app)
{
    int size = app.game.board.size;
    if (app.game.score.score > app.bests.get(size)) {
        app.bests.set(size, app.game.score.score);
        save_bests(app.bests);
    }
}

void start_game(App& app)
{
    GameOptions options;
    options.size = app.settings.grid;
    options.rescues = app.settings.rescues;
    options.deal_mode = app.settings.deal;
    options.allowed = pieces_for_shapes(app.settings.shapes);
    game_start(app.game, static_cast<uint32_t>(SDL_GetTicksNS()), options);
    app.previous_best = app.bests.get(app.settings.grid);
    app.undos_left = app.settings.undos;
    app.has_undo = false;
    app.undo_pressed = false;
    app.counted_score = -1;
    drop_saved_game(app);
    app.view = GameView{};
    app.over_pending = false;
    app.screen = Screen::Play;
}

void rescue_game(App& app)
{
    if (game_rescue(app.game)) {
        if (app.counted_score >= 0) {
            stats_revert_game_end(app.stats, app.counted_score);
            app.counted_score = -1;
            save_stats(app.stats);
        }
        sfx(app, Sound::Rescue);
        app.has_undo = false;
        app.view = GameView{};
        app.over_pending = false;
        app.screen = Screen::Play;
    }
}

void toggle_fullscreen(App& app)
{
    bool full = (SDL_GetWindowFlags(app.window) & SDL_WINDOW_FULLSCREEN) != 0;
    app.settings.fullscreen = !full;
    SDL_SetWindowFullscreen(app.window, !full);
    save_settings(app.settings);
}

void continue_game(App& app)
{
    app.game.rng.seed(static_cast<uint32_t>(SDL_GetTicksNS()));
    if (!load_saved_game(app.game, app.previous_best)) {
        app.menu.has_save = false;
        return;
    }
    app.view = GameView{};
    app.over_pending = false;
    app.undos_left = app.settings.undos;
    app.has_undo = false;
    app.undo_pressed = false;
    app.counted_score = -1;
    app.screen = Screen::Play;
}

void activate_menu(App& app, int which)
{
    if (which == kMenuContinue) {
        continue_game(app);
    } else if (which == kMenuPlay) {
        if (app.settings.tutorial_seen) {
            start_game(app);
        } else {
            app.tutorial_page = 0;
            app.tutorial_ms = 0;
            app.panel_pressed = -1;
            app.screen = Screen::Tutorial;
        }
    } else if (which == kMenuSettings) {
        app.reset_armed = false;
        app.panel_pressed = -1;
        app.screen = Screen::Settings;
    } else if (which == kMenuStats) {
        app.panel_pressed = -1;
        app.screen = Screen::Stats;
    } else {
        app.quit = true;
    }
}

void cancel_hand(App& app)
{
    app.view.hand_slot = -1;
    app.view.return_slot = -1;
}

void refresh_menu(App& app)
{
    app.menu.best_size = app.settings.grid;
    app.menu.best = app.bests.get(app.settings.grid);
}

void go_menu(App& app)
{
    persist_game(app);
    commit_best(app);
    cancel_hand(app);
    bool has_save = app.menu.has_save;
    app.menu = MenuView{};
    app.menu.has_save = has_save;
    refresh_menu(app);
    app.screen = Screen::Menu;
}

void sync_view(App& app)
{
    Lang lang = lang_from_index(app.settings.lang);
    app.menu.lang = lang;
    app.view.lang = lang;
    app.view.board = app.game.board;
    app.view.tray = app.game.tray;
    app.view.hold = app.game.hold;
    app.view.hold_enabled = app.settings.hold;
    app.view.undos_left = app.undos_left;
    app.view.undo_enabled = app.settings.undos > 0;
    app.view.undo_available = app.has_undo && !app.over_pending;
    app.view.undo_pressed = app.undo_pressed;
    app.view.score = app.game.score.score;
    app.view.streak = app.game.score.streak;
    app.view.best = std::max(app.bests.get(app.game.board.size), app.game.score.score);
}

int menu_hit(const App& app, float x, float y)
{
    for (int i = 0; i < 3; i++) {
        if (inside(menu_button(i).box, x, y)) {
            return i;
        }
    }
    if (app.menu.has_save && inside(menu_button(kMenuContinue).box, x, y)) {
        return kMenuContinue;
    }
    return inside(menu_button(kMenuStats).box, x, y) ? kMenuStats : -1;
}

void play_down(App& app, float x, float y)
{
    if (inside(kPauseButton, x, y)) {
        app.view.pause_pressed = true;
        return;
    }
    if (app.settings.undos > 0 && inside(kUndoButton, x, y)) {
        app.undo_pressed = true;
        return;
    }
    if (app.game.over) {
        return;
    }
    if (app.settings.hold && app.game.hold >= 0 && inside(kHoldRect, x, y)) {
        app.view.return_slot = -1;
        sfx(app, Sound::Pick);
        app.view.hand_slot = kTraySize;
        app.view.pointer_x = x;
        app.view.pointer_y = y - static_cast<float>(app.settings.lift);
        return;
    }
    for (int i = 0; i < kTraySize; i++) {
        Rect slot = {kTrayXs[i], kTrayY, kTraySlotSize, kTraySlotSize};
        if (app.game.tray[i] >= 0 && inside(slot, x, y)) {
            sfx(app, Sound::Pick);
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
    if (result.ok) {
        stats_on_move(app.stats, result.lines, app.game.score.streak);
    }
    if (result.lines > 0) {
        sfx_clear(app, result.lines, app.game.score.streak);
    } else {
        sfx(app, Sound::Place);
    }
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
        drop_saved_game(app);
    } else {
        persist_game(app);
    }
}

void undo_move(App& app)
{
    app.game = app.undo_game;
    sfx(app, Sound::Undo);
    app.has_undo = false;
    app.undos_left--;
    app.view = GameView{};
    app.over_pending = false;
    persist_game(app);
}

void play_up(App& app, float x, float y)
{
    if (app.undo_pressed) {
        app.undo_pressed = false;
        if (inside(kUndoButton, x, y) && app.has_undo && app.undos_left > 0 && !app.over_pending) {
            undo_move(app);
        }
        return;
    }
    if (app.view.pause_pressed) {
        app.view.pause_pressed = false;
        if (inside(kPauseButton, x, y)) {
            sfx(app, Sound::Click);
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
    const Piece& piece = piece_list()[game_piece_in_slot(app.game, slot)];
    Drop drop = find_drop(app.game.board, piece, app.view.pointer_x, app.view.pointer_y);
    app.view.hand_slot = -1;
    Rect reach = {kHoldRect.x - kHoldReach, kHoldRect.y - kHoldReach, kHoldRect.w + 2 * kHoldReach,
                  kHoldRect.h + 2 * kHoldReach};
    if (drop.valid) {
        app.undo_game = app.game;
        app.has_undo = true;
        finish_move(app, game_place(app.game, slot, drop.row, drop.col));
    } else if (app.settings.hold && slot < kTraySize && inside(reach, x, y) && game_swap_hold(app.game, slot)) {
        sfx(app, Sound::Hold);
        persist_game(app);
    } else {
        sfx(app, Sound::Invalid);
        app.view.return_slot = slot;
        app.view.return_t = 0;
    }
}

int over_hit(Gfx& g, float x, float y, bool chip, bool rescue)
{
    OverLayout l = over_layout(g, chip, rescue);
    if (rescue && inside(l.rescue, x, y)) {
        return 2;
    }
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
    for (int i = 0; i < l.count; i++) {
        if (inside(l.option[i], x, y)) {
            return i;
        }
    }
    return inside(l.back, x, y) ? kSettingsBack : -1;
}

void settings_activate(App& app, int hit)
{
    bool was_armed = app.reset_armed;
    app.reset_armed = false;
    int id = hit == kSettingsBack ? -1 : settings_layout().id[hit];
    if (id == kSettingGrid) {
        app.settings.grid = next_grid(app.settings.grid);
    } else if (id == kSettingLift) {
        app.settings.lift = next_lift(app.settings.lift);
    } else if (id == kSettingPieces) {
        app.panel_pressed = -1;
        app.screen = Screen::Pieces;
    } else if (id == kSettingDeal) {
        app.settings.deal = next_deal(app.settings.deal);
    } else if (id == kSettingVibration) {
        app.settings.vibration = !app.settings.vibration;
        if (app.settings.vibration) {
            haptics_pulse(pulse_for(Sound::Rescue));
        }
    } else if (id == kSettingLanguage) {
        app.settings.lang = next_language(app.settings.lang);
    } else if (id == kSettingSound) {
        app.settings.volume = next_volume(app.settings.volume);
        audio_set_volume(app.audio, app.settings.volume);
    } else if (id == kSettingUndo) {
        app.settings.undos = next_undos(app.settings.undos);
    } else if (id == kSettingHold) {
        app.settings.hold = !app.settings.hold;
    } else if (id == kSettingRescues) {
        app.settings.rescues = next_rescues(app.settings.rescues);
    } else if (id == kSettingFullscreen) {
        toggle_fullscreen(app);
    } else if (id == kSettingReset) {
        if (was_armed) {
            app.bests = Bests{};
            app.game.score = ScoreState{};
            save_bests(app.bests);
        } else {
            app.reset_armed = true;
        }
    } else {
        go_menu(app);
    }
    save_settings(app.settings);
}

bool tutorial_is_last(const App& app)
{
    return app.tutorial_page >= kTutorialPages - 1;
}

int tutorial_hit(const App& app, float x, float y)
{
    TutorialLayout l = tutorial_layout();
    if (inside(l.next, x, y)) {
        return 0;
    }
    return !tutorial_is_last(app) && inside(l.skip, x, y) ? 1 : -1;
}

void finish_tutorial(App& app)
{
    app.settings.tutorial_seen = true;
    save_settings(app.settings);
    start_game(app);
}

int pieces_hit(float x, float y)
{
    PiecesLayout l = pieces_layout();
    for (int i = 0; i < l.count; i++) {
        if (inside(l.card[i], x, y)) {
            return i;
        }
    }
    return inside(l.back, x, y) ? l.count : -1;
}

void pieces_activate(App& app, int hit)
{
    PiecesLayout l = pieces_layout();
    if (hit == l.count) {
        app.screen = Screen::Settings;
        return;
    }
    uint32_t next = app.settings.shapes ^ (uint32_t{1} << hit);
    if (next != 0) {
        app.settings.shapes = next;
        save_settings(app.settings);
    }
}

void pointer_down(App& app, Gfx& g, float x, float y)
{
    if (app.screen == Screen::Menu) {
        app.menu.focus = -1;
        app.menu.pressed = menu_hit(app, x, y);
    } else if (app.screen == Screen::Play) {
        play_down(app, x, y);
    } else if (app.screen == Screen::Pause) {
        app.panel_pressed = pause_hit(x, y);
    } else if (app.screen == Screen::Settings) {
        app.panel_pressed = settings_hit(x, y);
    } else if (app.screen == Screen::Pieces) {
        app.panel_pressed = pieces_hit(x, y);
    } else if (app.screen == Screen::Stats) {
        app.panel_pressed = inside(stats_layout().back, x, y) ? 0 : -1;
    } else if (app.screen == Screen::Tutorial) {
        app.panel_pressed = tutorial_hit(app, x, y);
    } else {
        bool chip = app.game.score.score > app.previous_best;
        app.panel_pressed = over_hit(g, x, y, chip, app.game.rescues_left > 0);
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
        int hit = menu_hit(app, x, y);
        int pressed = app.menu.pressed;
        app.menu.pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            activate_menu(app, hit);
        }
    } else if (app.screen == Screen::Play) {
        play_up(app, x, y);
    } else if (app.screen == Screen::Pause) {
        int hit = pause_hit(x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            if (hit == 0) {
                app.screen = Screen::Play;
            } else {
                go_menu(app);
            }
        }
    } else if (app.screen == Screen::Tutorial) {
        int hit = tutorial_hit(app, x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            if (hit == 1 || tutorial_is_last(app)) {
                finish_tutorial(app);
            } else {
                app.tutorial_page++;
                app.tutorial_ms = 0;
            }
        }
    } else if (app.screen == Screen::Stats) {
        int hit = inside(stats_layout().back, x, y) ? 0 : -1;
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            go_menu(app);
        }
    } else if (app.screen == Screen::Pieces) {
        int hit = pieces_hit(x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            pieces_activate(app, hit);
        }
    } else if (app.screen == Screen::Settings) {
        int hit = settings_hit(x, y);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            settings_activate(app, hit);
        }
    } else {
        bool chip = app.game.score.score > app.previous_best;
        int hit = over_hit(g, x, y, chip, app.game.rescues_left > 0);
        int pressed = app.panel_pressed;
        app.panel_pressed = -1;
        if (hit >= 0 && hit == pressed) {
            sfx(app, Sound::Click);
            if (hit == 2) {
                rescue_game(app);
            } else if (hit == 0) {
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
    bool back = key.key == SDLK_AC_BACK;
    if (key.key == SDLK_ESCAPE || back) {
        if (app.screen == Screen::Menu) {
            app.quit = back;
        } else if (app.screen == Screen::Play && !app.over_pending) {
            cancel_hand(app);
            app.view.pause_pressed = false;
            app.screen = Screen::Pause;
        } else if (app.screen == Screen::Pause) {
            app.screen = Screen::Play;
        } else if (app.screen == Screen::Pieces) {
            app.screen = Screen::Settings;
        } else if (app.screen == Screen::Tutorial) {
            finish_tutorial(app);
        } else if (app.screen == Screen::Over || app.screen == Screen::Settings || app.screen == Screen::Stats) {
            go_menu(app);
        }
        return;
    }
    if (app.screen != Screen::Menu) {
        return;
    }
    if (key.key == SDLK_TAB) {
        int order[5] = {kMenuContinue, kMenuPlay, kMenuSettings, kMenuStats, kMenuQuit};
        int first = app.menu.has_save ? 0 : 1;
        int count = 5 - first;
        int at = -1;
        for (int i = 0; i < count; i++) {
            if (order[first + i] == app.menu.focus) {
                at = i;
            }
        }
        bool backwards = (key.mod & SDL_KMOD_SHIFT) != 0;
        int next = at < 0 ? (backwards ? count - 1 : 0) : (at + (backwards ? count - 1 : 1)) % count;
        app.menu.focus = order[first + next];
    } else if (key.key == SDLK_RETURN || key.key == SDLK_KP_ENTER) {
        activate_menu(app, app.menu.focus < 0 ? kMenuPlay : app.menu.focus);
    }
}

bool SDLCALL lifecycle_watch(void* data, SDL_Event* event)
{
    if (event->type != SDL_EVENT_DID_ENTER_BACKGROUND && event->type != SDL_EVENT_TERMINATING) {
        return true;
    }
    App& app = *static_cast<App*>(data);
    if (app.screen == Screen::Play && !app.over_pending) {
        cancel_hand(app);
        app.view.pause_pressed = false;
        app.screen = Screen::Pause;
    }
    persist_game(app);
    commit_best(app);
    save_settings(app.settings);
    return true;
}

}

void app_init(App& app, Gfx& g, SDL_Window* window)
{
    SDL_AddEventWatch(lifecycle_watch, &app);
    app.window = window;
    if (titlebar_supported()) {
        SDL_SetWindowHitTest(window, titlebar_hit_test, nullptr);
    }
    apply_titlebar(app, g);
    app.bests = load_bests();
    app.stats = load_stats();
    app.settings = load_settings();
    audio_init(app.audio, app.settings.volume);
    if (app.settings.fullscreen && fullscreen_supported()) {
        SDL_SetWindowFullscreen(window, true);
    }
    refresh_menu(app);
    Game probe;
    int previous = 0;
    app.menu.has_save = load_saved_game(probe, previous);
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
    case SDL_EVENT_RENDER_DEVICE_RESET:
    case SDL_EVENT_RENDER_TARGETS_RESET:
        gfx_reset(g);
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
    if (app.screen == Screen::Tutorial) {
        app.tutorial_ms += dt_ms;
        return;
    }
    if (app.screen != Screen::Play && app.screen != Screen::Pause) {
        return;
    }
    if (app.screen == Screen::Pause) {
        return;
    }
    app.play_ms += dt_ms;
    while (app.play_ms >= 1000.0f) {
        app.play_ms -= 1000.0f;
        app.stats.seconds++;
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
            stats_on_game_end(app.stats, app.game.score.score);
            app.counted_score = app.game.score.score;
            save_stats(app.stats);
            app.panel_pressed = -1;
            sfx(app, Sound::GameOver);
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
        pv.lang = lang_from_index(app.settings.lang);
        pv.board = app.game.board;
        pv.score = app.game.score.score;
        pv.pressed = app.panel_pressed;
        draw_pause(g, pv);
    } else if (app.screen == Screen::Tutorial) {
        TutorialView tv;
        tv.lang = lang_from_index(app.settings.lang);
        tv.page = app.tutorial_page;
        tv.seconds = app.tutorial_ms / 1000.0f;
        tv.pressed = app.panel_pressed;
        draw_tutorial(g, tv);
    } else if (app.screen == Screen::Stats) {
        StatsView sv;
        sv.lang = lang_from_index(app.settings.lang);
        sv.stats = app.stats;
        sv.bests = app.bests;
        sv.pressed = app.panel_pressed;
        draw_stats(g, sv);
    } else if (app.screen == Screen::Pieces) {
        PiecesView pv;
        pv.lang = lang_from_index(app.settings.lang);
        pv.shapes = app.settings.shapes;
        pv.pressed = app.panel_pressed;
        draw_pieces(g, pv);
    } else if (app.screen == Screen::Settings) {
        SettingsView sv;
        sv.lang = lang_from_index(app.settings.lang);
        sv.grid = app.settings.grid;
        sv.lift = app.settings.lift;
        sv.rescues = app.settings.rescues;
        sv.undos = app.settings.undos;
        sv.hold = app.settings.hold;
        sv.volume = app.settings.volume;
        sv.vibration = app.settings.vibration;
        sv.deal = app.settings.deal;
        sv.shapes_on = std::popcount(app.settings.shapes);
        sv.shapes_total = shape_count();
        sv.fullscreen = app.settings.fullscreen;
        sv.reset_armed = app.reset_armed;
        sv.pressed = app.panel_pressed;
        draw_settings(g, sv);
    } else {
        OverView ov;
        ov.lang = lang_from_index(app.settings.lang);
        ov.board = app.game.board;
        ov.score = app.game.score.score;
        ov.previous_best = app.previous_best;
        ov.new_best = app.game.score.score > app.previous_best;
        ov.rescues_left = app.game.rescues_left;
        ov.pressed = app.panel_pressed;
        draw_game_over(g, ov);
    }
    SDL_SetRenderViewport(g.renderer, nullptr);
}

void app_shutdown(App& app)
{
    audio_shutdown(app.audio);
    persist_game(app);
    commit_best(app);
    save_settings(app.settings);
}
