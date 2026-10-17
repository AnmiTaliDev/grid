// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#include "i18n.hpp"

#include <cstring>

namespace {

struct Entry {
    const char* en;
    const char* ru;
};

const Entry kTable[] = {
    {"Play", "Играть"},
    {"Continue", "Продолжить"},
    {"Settings", "Настройки"},
    {"Stats", "Статистика"},
    {"Quit", "Выход"},
    {"Best", "Рекорд"},
    {"Previous best", "Прежний рекорд"},
    {"New best", "Новый рекорд"},
    {"No moves left", "Ходов не осталось"},
    {"Rescue", "Спасение"},
    {"left", "ост."},
    {"Again", "Заново"},
    {"Menu", "Меню"},
    {"Paused", "Пауза"},
    {"Resume", "Продолжить"},
    {"Back", "Назад"},
    {"streak", "серия"},
    {"Undo", "Отмена"},
    {"Hold", "Запас"},
    {"on", "вкл"},
    {"off", "выкл"},
    {"none", "нет"},
    {"Grid size", "Сетка"},
    {"Piece lift", "Подъём фигуры"},
    {"Fullscreen", "Полный экран"},
    {"Deal", "Раздача"},
    {"Pieces", "Фигуры"},
    {"Rescues", "Спасения"},
    {"Undo", "Отмена"},
    {"Hold", "Запас"},
    {"Sound", "Звук"},
    {"Vibration", "Вибрация"},
    {"Language", "Язык"},
    {"Reset bests", "Сбросить рекорды"},
    {"Confirm reset", "Подтвердить сброс"},
    {"random", "случайная"},
    {"fitting", "подходящая"},
    {"dynamic", "динамичная"},
    {"Pieces", "Фигуры"},
    {"Statistics", "Статистика"},
    {"Games played", "Сыграно партий"},
    {"Average score", "Средний счёт"},
    {"Longest streak", "Лучшая серия"},
    {"Lines cleared", "Очищено линий"},
    {"Pieces placed", "Поставлено фигур"},
    {"Time played", "Время в игре"},
    {"h", "ч"},
    {"m", "мин"},
    {"s", "с"},
    {"How to play", "Как играть"},
    {"Next", "Далее"},
    {"Start", "Начать"},
    {"Skip", "Пропустить"},
    {"takes back the last move", "возвращает последний ход"},
    {"keeps a piece for later", "хранит фигуру на потом"},
    {"gives new pieces when stuck", "даёт фигуры, если ходов нет"},
};

static_assert(sizeof(kTable) / sizeof(kTable[0]) == static_cast<size_t>(Str::Count),
              "every Str needs an English and a Russian text");

}

const char* tr(Lang lang, Str id)
{
    const Entry& e = kTable[static_cast<size_t>(id)];
    return lang == Lang::Russian ? e.ru : e.en;
}

Lang lang_from_index(int index)
{
    return index == 1 ? Lang::Russian : Lang::English;
}

Lang detect_language(const char* preferred)
{
    if (preferred && std::strncmp(preferred, "ru", 2) == 0) {
        return Lang::Russian;
    }
    return Lang::English;
}

std::string language_name(Lang lang)
{
    return lang == Lang::Russian ? "Русский" : "English";
}

int plural_form_ru(int count)
{
    int n = count < 0 ? -count : count;
    int last = n % 10;
    int last_two = n % 100;
    if (last == 1 && last_two != 11) {
        return 0;
    }
    if (last >= 2 && last <= 4 && (last_two < 12 || last_two > 14)) {
        return 1;
    }
    return 2;
}

std::string lines_text(Lang lang, int count)
{
    if (lang == Lang::English) {
        return count == 1 ? "1 line" : std::to_string(count) + " lines";
    }
    const char* forms[3] = {"линия", "линии", "линий"};
    return std::to_string(count) + " " + forms[plural_form_ru(count)];
}
