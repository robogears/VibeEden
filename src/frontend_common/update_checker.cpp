// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// Copyright Citra Emulator Project / Azahar Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#ifdef NIGHTLY_BUILD
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#endif

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include "common/net/net.h"
#include "common/scm_rev.h"
#include "update_checker.h"

#include "common/logging.h"

namespace {
// Numeric, "v"-tolerant version comparison (per the updater spec). Avoids the lexicographic
// pitfalls of string comparison (e.g. "0.1.10" < "0.1.2") and ignores a leading "v".
std::vector<int> ParseVersion(std::string v) {
    if (!v.empty() && (v.front() == 'v' || v.front() == 'V')) {
        v.erase(0, 1);
    }
    std::vector<int> parts;
    std::stringstream ss(v);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        try {
            parts.push_back(std::stoi(segment));
        } catch (...) {
            parts.push_back(0);
        }
    }
    return parts;
}

bool IsNewerVersion(const std::string& remote, const std::string& current) {
    const auto r = ParseVersion(remote);
    const auto c = ParseVersion(current);
    const std::size_t n = std::max(r.size(), c.size());
    for (std::size_t i = 0; i < n; ++i) {
        const int a = i < r.size() ? r[i] : 0;
        const int b = i < c.size() ? c[i] : 0;
        if (a > b) {
            return true;
        }
        if (a < b) {
            return false;
        }
    }
    return false;
}
} // namespace

UpdateChecker::Result UpdateChecker::GetUpdate() {
    const auto latest = Common::Net::GetLatestRelease();
    // A null result here specifically means the request failed (no network, timeout, bad JSON) -
    // GetLatestRelease never returns null to mean "no newer release". Report that distinctly so the
    // UI does not tell the user they are up to date when the check actually failed.
    if (!latest) return {Status::CheckFailed, std::nullopt};

    LOG_INFO(Frontend, "Received update {}", latest->title);

#ifdef NIGHTLY_BUILD
    std::vector<std::string> result;

    boost::split(result, latest->tag, boost::is_any_of("."));
    if (result.size() != 2)
        return {Status::CheckFailed, std::nullopt};

    const std::string tag = result[1];

    boost::split(result, std::string{Common::g_build_version}, boost::is_any_of("-"));
    if (result.empty())
        return {Status::CheckFailed, std::nullopt};

    const std::string build = result[0];
#else
    const std::string tag = latest->tag;
    const std::string build = Common::g_build_version;
#endif

    if (IsNewerVersion(tag, build))
        return {Status::UpdateAvailable, latest};

    return {Status::UpToDate, std::nullopt};
}
