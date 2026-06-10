// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// Copyright Citra Emulator Project / Azahar Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <optional>
#include "common/net/net.h"

namespace UpdateChecker {

enum class Status {
    UpToDate,        // the check succeeded and there is no newer release
    UpdateAvailable, // the check succeeded and a newer release is available (in Result::release)
    CheckFailed,     // the check could not complete (no network, timeout, parse error)
};

struct Result {
    Status status;
    std::optional<Common::Net::Release> release;
};

Result GetUpdate();

} // namespace UpdateChecker
