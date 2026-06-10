// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

#include "common/logging.h"
#include "common/scope_exit.h"
#include "common/param_package.h"
#include "common/settings_input.h"
#include "core/core.h"
#include "hid_core/frontend/emulated_controller.h"
#include "hid_core/hid_core.h"
#include "hid_core/hid_types.h"
#include "input_common/main.h"
#include "qt_common/config/uisettings.h"

#include "yuzu/steam_controller_mapper.h"

namespace {

constexpr std::size_t MAX_PLAYERS = 8;

/// Sentinel used when a pad has no Steam/SDL player index; sorts such pads to the end.
constexpr int NO_INDEX = std::numeric_limits<int>::max();

int OrderKey(const Common::ParamPackage& device) {
    const int index = device.Get("player_index", -1);
    return index < 0 ? NO_INDEX : index;
}

/// Collect the currently connected SDL gamepads, ordered by Steam/SDL player index.
std::vector<Common::ParamPackage> GatherOrderedPads(
    const InputCommon::InputSubsystem& input_subsystem) {
    std::vector<Common::ParamPackage> pads;
    for (const auto& device : input_subsystem.GetInputDevices()) {
        // Only physical gamepads carry Steam's ordering; skip keyboard/mouse/tas/etc.
        if (device.Get("engine", "") != "sdl") {
            continue;
        }
        if (!device.Has("guid") || !device.Has("port")) {
            continue;
        }
        // Skip the synthetic "dual Joy-Con" combo entries (they carry a second guid); each
        // physical pad is already represented by its own single entry. Steam Deck/Steam Input
        // never produces these, so this only matters for stray bare Joy-Cons on desktop.
        if (device.Has("guid2")) {
            continue;
        }
        pads.push_back(device);
    }

    // Steam Input is the source of truth on the Steam Deck: when any Steam virtual gamepad is
    // present, bind ONLY those and ignore raw physical devices (DualShock, keyboard, etc.)
    // entirely. Off Steam (e.g. desktop development) we fall back to plain SDL gamepads so the
    // feature stays testable.
    const bool any_steam_virtual =
        std::any_of(pads.begin(), pads.end(), [](const Common::ParamPackage& p) {
            return p.Get("steam_virtual", 0) != 0;
        });
    if (any_steam_virtual) {
        pads.erase(std::remove_if(pads.begin(), pads.end(),
                                  [](const Common::ParamPackage& p) {
                                      return p.Get("steam_virtual", 0) == 0;
                                  }),
                   pads.end());
    }

    std::stable_sort(pads.begin(), pads.end(),
                     [](const Common::ParamPackage& a, const Common::ParamPackage& b) {
                         const int ka = OrderKey(a);
                         const int kb = OrderKey(b);
                         if (ka != kb) {
                             return ka < kb;
                         }
                         // Deterministic tie-break for pads with no Steam index (desktop fallback),
                         // so their slot order does not depend on hash-map iteration order.
                         const auto ga = a.Get("guid", "");
                         const auto gb = b.Get("guid", "");
                         if (ga != gb) {
                             return ga < gb;
                         }
                         return a.Get("port", 0) < b.Get("port", 0);
                     });
    return pads;
}

std::string MakeSignature(const std::vector<Common::ParamPackage>& pads) {
    std::string sig;
    for (const auto& pad : pads) {
        sig += pad.Get("guid", "");
        sig += ':';
        sig += std::to_string(pad.Get("port", 0));
        sig += ':';
        sig += std::to_string(pad.Get("player_index", -1));
        sig += '|';
    }
    return sig;
}

} // namespace

SteamControllerMapper::SteamControllerMapper(Core::System& system_,
                                             InputCommon::InputSubsystem& input_subsystem_)
    : system{system_}, input_subsystem{input_subsystem_} {}

SteamControllerMapper::~SteamControllerMapper() = default;

void SteamControllerMapper::Invalidate() {
    // Any value the next signature cannot equal forces a rebind.
    last_signature = "\x01invalidate";
}

bool SteamControllerMapper::Sync(bool emulation_active) {
    // Respect the user's toggle (default on). When disabled, clear the signature so that
    // re-enabling forces a fresh bind even if the controller set is unchanged.
    if (!UISettings::values.auto_map_controllers.GetValue()) {
        last_signature.clear();
        return false;
    }

    const auto pads = GatherOrderedPads(input_subsystem);
    const auto signature = MakeSignature(pads);
    if (signature == last_signature) {
        return false;
    }
    last_signature = signature;

    auto& hid_core = system.HIDCore();

    // Pro Controller (Fullkey) must be an allowed style for Connect() to succeed. Outside of a
    // running game nothing constrains the supported set, so widen it; while a game is running we
    // leave its chosen styles alone (Pro Controller is accepted by virtually every title anyway).
    if (!emulation_active) {
        hid_core.SetSupportedStyleTag({Core::HID::NpadStyleSet::All});
    }

    // Assign each pad to a player slot. Under Steam Input we follow the pad's own player_index
    // exactly, so dropping one pad never shuffles the others down a slot: each survivor keeps its
    // slot, and the dropped slot simply goes quiet (the disconnect no-op). Only the desktop
    // fallback (pads with no Steam index) is packed densely into the lowest free slots.
    std::array<const Common::ParamPackage*, MAX_PLAYERS> slot_device{};
    std::vector<const Common::ParamPackage*> unindexed;
    for (const auto& pad : pads) {
        const int idx = pad.Get("player_index", -1);
        if (idx >= static_cast<int>(MAX_PLAYERS)) {
            // More controllers than the Switch's 8 player slots: ignore the extras rather than
            // letting a 9th-ordered pad get packed onto a low player slot.
            continue;
        }
        if (idx >= 0 && slot_device[idx] == nullptr) {
            slot_device[idx] = &pad;
        } else {
            unindexed.push_back(&pad);
        }
    }
    for (const auto* const pad : unindexed) {
        for (std::size_t s = 0; s < MAX_PLAYERS; ++s) {
            if (slot_device[s] == nullptr) {
                slot_device[s] = pad;
                break;
            }
        }
    }

    // Enter "configuration" mode on every controller so connection/type changes are staged and
    // committed atomically on DisableAllControllerConfiguration() (the same dance the settings
    // dialog uses, safe even mid-game). The scope guard guarantees we leave configuration mode even
    // on an early return or exception - otherwise HID could get stuck suppressing all input.
    hid_core.EnableAllControllerConfiguration();
    bool config_active = true;
    SCOPE_EXIT {
        if (config_active) {
            hid_core.DisableAllControllerConfiguration();
        }
    };

    for (std::size_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        const Common::ParamPackage* const device = slot_device[slot];
        if (device == nullptr) {
            // No pad currently maps to this slot. Leave the emulated controller exactly as-is: a
            // physical disconnect must be a no-op (no freeze, no applet), and an already-bound
            // player keeps playing when its pad returns.
            continue;
        }

        auto* const controller = hid_core.GetEmulatedControllerByIndex(slot);

        // Apply the standard SDL gamepad -> Switch (Pro Controller) mapping. The button and stick
        // getters always emit a full mapping, so we deliberately do NOT pre-clear those (each
        // SetParam triggers a ReloadInput(), and clearing first would roughly double that churn).
        for (const auto& [index, param] : input_subsystem.GetButtonMappingForDevice(*device)) {
            controller->SetButtonParam(static_cast<std::size_t>(index), param);
        }
        for (const auto& [index, param] : input_subsystem.GetAnalogMappingForDevice(*device)) {
            controller->SetStickParam(static_cast<std::size_t>(index), param);
        }
        // Motion is the exception: GetMotionMappingForDevice only emits entries for a motion-capable
        // pad, so clear it first or a previous pad's motion binding could linger on a rebind.
        for (std::size_t i = 0; i < Settings::NativeMotion::NumMotions; ++i) {
            controller->SetMotionParam(i, {});
        }
        for (const auto& [index, param] : input_subsystem.GetMotionMappingForDevice(*device)) {
            controller->SetMotionParam(static_cast<std::size_t>(index), param);
        }

        controller->SetNpadStyleIndex(Core::HID::NpadStyleIndex::Fullkey);
        controller->Connect();
    }

    // Commit staged temporaries to the live controllers, then disarm the guard.
    hid_core.DisableAllControllerConfiguration();
    config_active = false;

    // Persist the result into Settings so it survives a restart / matches the saved config.
    for (std::size_t slot = 0; slot < MAX_PLAYERS; ++slot) {
        hid_core.GetEmulatedControllerByIndex(slot)->SaveCurrentConfig();
    }

    LOG_INFO(Input, "Steam controller auto-map: bound {} controller(s) to players in Steam order",
             std::min(pads.size(), MAX_PLAYERS));
    return true;
}
