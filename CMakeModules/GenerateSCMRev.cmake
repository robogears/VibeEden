# SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2019 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later

# generate git/build information
include(GetSCMRev)

function(get_timestamp _var)
    string(TIMESTAMP timestamp UTC)
    set(${_var} "${timestamp}" PARENT_SCOPE)
endfunction()

get_timestamp(BUILD_DATE)

if (DEFINED GIT_RELEASE)
    set(BUILD_VERSION "${GIT_TAG}")
    set(GIT_REFSPEC "${GIT_RELEASE}")
    set(IS_DEV_BUILD false)
else()
    string(SUBSTRING ${GIT_COMMIT} 0 10 BUILD_VERSION)
    set(BUILD_VERSION "${BUILD_VERSION}-${GIT_REFSPEC}")
    set(IS_DEV_BUILD true)
endif()

if (NIGHTLY_BUILD)
    set(IS_NIGHTLY_BUILD true)
else()
    set(IS_NIGHTLY_BUILD false)
endif()

# VibeEden fork version: drives the in-app updater's comparison against the
# robogears/VibeEden GitHub releases. Bump per release, or override with -DVIBEEDEN_VERSION=x.y.z.
if (NOT DEFINED VIBEEDEN_VERSION)
    set(VIBEEDEN_VERSION "0.2.2")
endif()
set(BUILD_VERSION "${VIBEEDEN_VERSION}")
set(IS_DEV_BUILD false)

set(GIT_DESC ${BUILD_VERSION})

# Generate cpp with Git revision from template

# NOTE: these STABLE_* vars are NOT dead - they are read by the BCAT builtin-news service
# (src/core/hle/service/bcat/news/builtin_news.cpp), so a stock build fetches emulated "news" from
# upstream Eden's Forgejo. Leave them Forgejo-shaped; a naive GitHub swap would 404. The
# NIGHTLY_BUILD branch below is inherited-but-unused for VibeEden (we ship via GitHub Releases).
set(BUILD_AUTO_UPDATE_STABLE_REPO "eden-emu/eden")
set(BUILD_AUTO_UPDATE_STABLE_API "git.eden-emu.dev")
set(BUILD_AUTO_UPDATE_STABLE_API_PATH "/api/v1/repos/")

if (NIGHTLY_BUILD)
    set(BUILD_AUTO_UPDATE_API_PATH "/latest/release.json")
    set(BUILD_AUTO_UPDATE_WEBSITE "https://git.eden-emu.dev")
    set(BUILD_AUTO_UPDATE_API "nightly.eden-emu.dev")
    set(BUILD_AUTO_UPDATE_REPO "eden-ci/nightly")
    set(REPO_NAME "Eden Nightly")
else()
    # VibeEden ships via GitHub Releases (robogears/VibeEden). Point the updater at the GitHub
    # REST API; Release::FromJson already understands the GitHub releases JSON (assets[].url).
    set(BUILD_AUTO_UPDATE_API_PATH "/repos/robogears/VibeEden/releases/latest")
    set(BUILD_AUTO_UPDATE_WEBSITE "https://github.com")
    set(BUILD_AUTO_UPDATE_API "api.github.com")
    set(BUILD_AUTO_UPDATE_REPO "robogears/VibeEden")
    set(REPO_NAME "VibeEden")
endif()

set(BUILD_ID ${GIT_REFSPEC})
set(BUILD_FULLNAME "${REPO_NAME} ${BUILD_VERSION} ")
set(CXX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

configure_file(scm_rev.cpp.in scm_rev.cpp @ONLY)
