// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "tutorial.hpp"

#include <algorithm>
#include <cmath>

const std::vector<std::string>& tutorial_caption(int page, Lang lang)
{
    static const std::vector<std::vector<std::string>> pages = {
        {"Drag a piece from the tray", "onto the board.", "Pieces cannot be rotated."},
        {"Fill a whole row or column", "to clear it. Clear lines in a row", "to build a streak."},
        {"The game ends when no piece fits.", "Undo, Hold and Rescue can help.", "All of them are set in Settings."},
    };
    static const std::vector<std::vector<std::string>> pages_ru = {
        {"Перетащите фигуру из трея", "на поле.", "Фигуры нельзя поворачивать."},
        {"Заполните целый ряд или столбец,", "чтобы очистить его. Очищайте линии", "подряд, чтобы набрать серию."},
        {"Игра заканчивается, когда ходов нет.", "Отмена, запас и спасение помогут.", "Всё это включается в настройках."},
    };
    size_t index = static_cast<size_t>(std::clamp(page, 0, kTutorialPages - 1));
    return lang == Lang::Russian ? pages_ru[index] : pages[index];
}

float tutorial_loop(float seconds)
{
    if (seconds < 0) {
        return 0;
    }
    return std::fmod(seconds, kTutorialPeriod);
}

float tutorial_ease(float t, float start, float end)
{
    if (end <= start) {
        return t >= end ? 1.0f : 0.0f;
    }
    float x = std::clamp((t - start) / (end - start), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}
