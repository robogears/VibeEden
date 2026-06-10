// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "common/common_types.h"

namespace Common::Net {

typedef struct {
    std::string name;
    std::string url;
    std::string path;
    std::string filename;
    std::string sha256; // expected lowercase-hex SHA-256 of the asset; empty if unknown
} Asset;

typedef struct Release {
    std::string title;
    std::string body;
    std::string tag;
    std::string base_download_url;
    std::string html_url;
    std::string host;

    std::vector<std::string> assets;
    // Maps an asset's browser_download_url to its expected lowercase-hex SHA-256 (parsed from
    // GitHub's per-asset "digest" field), when the host provides it.
    std::unordered_map<std::string, std::string> asset_sha256;

    u64 id;
    u64 published;
    bool prerelease;

    // Get the relevant list of assets for the current platform.
    std::vector<Asset> GetPlatformAssets() const;

    static std::optional<Release> FromJson(const nlohmann::json& json, const std::string &host, const std::string& repo);
    static std::optional<Release> FromJson(const std::string_view& json, const std::string &host, const std::string& repo);
    static std::vector<Release> ListFromJson(const nlohmann::json &json, const std::string &host, const std::string &repo);
    static std::vector<Release> ListFromJson(const std::string_view &json, const std::string &host, const std::string &repo);
} Release;

// Make a request via httplib, and return the response body if applicable.
std::optional<std::string> MakeRequest(const std::string &url, const std::string &path);

// Get all of the latest stable releases.
std::vector<Release> GetReleases();

// Get all of the latest stable releases as text.
std::optional<std::string> GetReleasesBody();

// Get the latest release of the current channel.
std::optional<Release> GetLatestRelease();

}
