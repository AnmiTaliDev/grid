// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <string>

enum class Lang {
    English,
    Russian,
};

constexpr int kLangCount = 2;

enum class Str {
    Play,
    Continue,
    Settings,
    Stats,
    Quit,
    Best,
    PreviousBest,
    NewBest,
    NoMovesLeft,
    Rescue,
    Left,
    Again,
    Menu,
    Paused,
    Resume,
    Back,
    Streak,
    Undo,
    Hold,
    On,
    Off,
    None,
    SetGrid,
    SetLift,
    SetFullscreen,
    SetDeal,
    SetPieces,
    SetRescues,
    SetUndo,
    SetHold,
    SetSound,
    SetVibration,
    SetLanguage,
    ResetBests,
    ConfirmReset,
    DealRandom,
    DealFitting,
    DealDynamic,
    PiecesTitle,
    StatsTitle,
    GamesPlayed,
    AverageScore,
    LongestStreak,
    LinesCleared,
    PiecesPlaced,
    TimePlayed,
    UnitHour,
    UnitMinute,
    UnitSecond,
    HowToPlay,
    Next,
    Start,
    Skip,
    UndoHelp,
    HoldHelp,
    RescueHelp,
    Count,
};

const char* tr(Lang lang, Str id);
Lang lang_from_index(int index);
Lang detect_language(const char* preferred);
std::string language_name(Lang lang);
std::string lines_text(Lang lang, int count);
int plural_form_ru(int count);
