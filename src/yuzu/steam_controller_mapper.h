// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace Core {
class System;
}

namespace InputCommon {
class InputSubsystem;
}

/**
 * Steam Deck / Steam Input controller auto-mapper.
 *
 * Goal: make controllers "just work" without any trip through the controller settings.
 * Steam (the Steam Deck / Big Picture "rearrange controllers" UI) is treated as the single
 * source of truth for which physical pad is Player 1, 2, 3, ... It exposes that ordering via
 * each pad's SDL player index (see SDLJoystick::GetPlayerIndex). This class enumerates the
 * connected SDL gamepads, sorts them by that player index, and binds Player k+1 to the pad at
 * Steam slot k using the standard Switch Pro Controller mapping derived from SDL's gamepad DB.
 *
 * It runs headlessly (no dialog) on startup and whenever the set of connected controllers
 * changes, so connecting/disconnecting a pad is reflected immediately. Under Steam Input every
 * virtual pad shares the same GUID/name, so the player index is the only stable discriminator -
 * which is exactly why honoring Steam's order is both the friendliest and the only robust option.
 */
class SteamControllerMapper {
public:
    SteamControllerMapper(Core::System& system_, InputCommon::InputSubsystem& input_subsystem_);
    ~SteamControllerMapper();

    /**
     * Re-evaluates the connected controllers and, if the set/order changed since the last call,
     * (re)binds every player slot to match Steam's ordering. Cheap to call frequently: it only
     * does work when the controller signature actually changes.
     *
     * @param emulation_active Whether a game is currently running. When false we may widen the
     *                         supported controller styles so the Pro Controller binding connects;
     *                         when true we leave the running game's supported styles untouched.
     * @return true if a (re)bind was performed this call.
     */
    bool Sync(bool emulation_active);

    /// Forces the next Sync() to rebind even if the controller set is unchanged.
    void Invalidate();

private:
    Core::System& system;
    InputCommon::InputSubsystem& input_subsystem;
    std::string last_signature;
};
