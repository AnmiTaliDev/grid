// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>

#include "piece.hpp"

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

constexpr Color rgb(uint32_t hex, uint8_t alpha = 255)
{
    return {static_cast<uint8_t>(hex >> 16), static_cast<uint8_t>(hex >> 8), static_cast<uint8_t>(hex), alpha};
}

constexpr Color with_alpha(Color c, float alpha)
{
    return {c.r, c.g, c.b, static_cast<uint8_t>(alpha * 255.0f + 0.5f)};
}

namespace palette {

constexpr Color kNight = rgb(0x101C26);
constexpr Color kSlate = rgb(0x172733);
constexpr Color kSlateRing = rgb(0x223646);
constexpr Color kBedEdge = rgb(0x0A131A);
constexpr Color kWell = rgb(0x0C1620);
constexpr Color kTraySlot = rgb(0x121F29);
constexpr Color kButtonFace = rgb(0x1C3040);
constexpr Color kBone = rgb(0xEFE9DC);
constexpr Color kBoneEdge = rgb(0xB9B09C);
constexpr Color kDim = rgb(0x8AA1B1);
constexpr Color kInk = rgb(0x101C26);
constexpr Color kFocus = rgb(0x6FD1AA);
constexpr Color kPanelTitle = rgb(0x3A4A56);
constexpr Color kPanelText = rgb(0x55656F);
constexpr Color kPanelSecondary = rgb(0xDDD5C3);
constexpr Color kPanelPrimaryEdge = rgb(0x05090D);
constexpr Color kSaffronFace = rgb(0xF3B93F);
constexpr Color kSaffronEdge = rgb(0xB07F14);
constexpr Color kPersimmonFace = rgb(0xEE6A4C);
constexpr Color kPersimmonEdge = rgb(0xA8412A);

constexpr Color kGlazeFace[6] = {
    rgb(0xEE6A4C), rgb(0xF3B93F), rgb(0x6FD1AA), rgb(0x4C7CF0), rgb(0xC07AE0), rgb(0xEFE9DC),
};
constexpr Color kGlazeEdge[6] = {
    rgb(0xA8412A), rgb(0xB07F14), rgb(0x3A9A74), rgb(0x2A4BB0), rgb(0x8445A6), rgb(0xB9B09C),
};

}

constexpr Color dim_color(Color c)
{
    float r = 0.48845f * c.r + 0.46475f * c.g + 0.04680f * c.b;
    float g = 0.13845f * c.r + 0.81475f * c.g + 0.04680f * c.b;
    float b = 0.13845f * c.r + 0.46475f * c.g + 0.39680f * c.b;
    return {static_cast<uint8_t>(r * 0.6f + 0.5f), static_cast<uint8_t>(g * 0.6f + 0.5f),
            static_cast<uint8_t>(b * 0.6f + 0.5f), c.a};
}
