// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

constexpr int kScreenW = 540;
constexpr int kScreenH = 960;

constexpr float kCellPitch = 58.0f;
constexpr float kCellSize = 54.0f;
constexpr float kBoardY = 228.0f;
constexpr float kBedY = 216.0f;

constexpr float bed_width(int size)
{
    return kCellPitch * static_cast<float>(size) + 20.0f;
}

constexpr float bed_x(int size)
{
    return 270.0f - bed_width(size) / 2;
}

constexpr float board_x(int size)
{
    return bed_x(size) + 12.0f;
}

constexpr float kTrayY = 748.0f;
constexpr float kTrayXs[3] = {28.0f, 196.0f, 364.0f};
constexpr float kTraySlotSize = 148.0f;

