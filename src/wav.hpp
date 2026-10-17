// SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <cstdint>
#include <vector>

std::vector<uint8_t> encode_wav(const std::vector<float>& samples);
std::vector<int16_t> quantize_samples(const std::vector<float>& samples);
