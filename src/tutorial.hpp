// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <string>
#include <vector>

constexpr int kTutorialPages = 3;
constexpr float kTutorialPeriod = 3.2f;

const std::vector<std::string>& tutorial_caption(int page);
float tutorial_loop(float seconds);
float tutorial_ease(float t, float start, float end);
