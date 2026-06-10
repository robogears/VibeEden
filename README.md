<!--
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later
-->
<!-- lang: en-GB -->

<h1 align="center">
  <br>
  <a href="https://git.eden-emu.dev/eden-emu/eden"><img src="./dist/qt_themes/default/icons/256x256/eden.png" alt="Eden" width="200"></a>
  <br>
  <b>Eden</b>
  <br>
</h1>

<h4 align="center"><b>Eden</b> is a free and opensource (FOSS) Switch 1 emulator, derived from Yuzu and Sudachi - started by developer Camille LaVey.
It's written in C++ with portability in mind, with builds for Windows, Linux, macOS, Android, FreeBSD and more.
</h4>


## Disclaimer

This fork was **vibecoded**, so the code may not be perfect.

There may be bugs, messy parts, unfinished features, or security issues.

Use it at your own risk. Review and test it before using it for anything important.

## VibeEden — Steam Deck controller rework

**VibeEden** is a **VIBE CODED** Steam-Deck-focused fork of Eden. Stock Switch emulators make you assign a physical device to every player and remap buttons by hand — clumsy on a Deck, where **Steam already manages controllers**. VibeEden replaces that flow so controllers *just work* under SteamOS.

### Controllers follow Steam, automatically
- **Steam owns the order.** Whatever order Steam assigns your controllers (Steam Input's *Rearrange Controllers* / player index) is exactly the order VibeEden binds them — the pad in Steam slot 1 becomes Player 1, slot 2 becomes Player 2, and so on. No per-player device pickers.
- **Zero manual setup.** Connect a controller — Bluetooth, USB, the Deck's built-in pad, anything Steam exposes — and it is bound to the next free player as a **Pro Controller** instantly. No button-mapping screens, no profiles.
- **Steam virtual input only.** Under Steam Input every pad reports the same identity, so VibeEden binds *only* the Steam virtual gamepads and ignores raw device types (DualShock, keyboard, etc.) — the one model that is actually reliable on a Deck. Off Steam (on a desktop) it falls back to plain SDL gamepads so it stays usable there too.
- **Disconnects are a no-op.** Lose a controller mid-game (battery dies, a Bluetooth blip) and **nothing happens** — no pause, no popup, the game keeps running. Reconnect and you are back in control on the same player slot, with none of the Switch-style "controller disconnected" freeze.

### A Deck-friendly controller prompt
When a game genuinely needs *more* controllers than are connected, VibeEden shows a clean, dark, Switch-style prompt instead of the busy stock dialog (no dropdowns, vibration, motion, or profile clutter) and **auto-resumes the instant** the requirement is met. The full manual per-player editor is still one click away under **Advanced** for unusual setups.

### Self-updating
VibeEden checks its own GitHub releases and can update itself in place: it downloads the new AppImage, **verifies its SHA-256** against GitHub's per-asset digest *before* the file is ever made executable, then swaps it in and relaunches. A **Check for Updates** button lives in the status bar.

### Running it on a Steam Deck
1. Download `Eden.AppImage` from the [latest release](https://github.com/robogears/VibeEden/releases/latest) and mark it executable (`chmod +x Eden.AppImage`).
2. Add it to Steam as a non-Steam game.
3. In its per-game controller settings, enable **Steam Input** and arrange your controllers in Steam's order.
4. Launch from Game Mode — controllers map themselves.

Requires a current SteamOS / Linux (glibc 2.38+) with working Vulkan drivers.

> VibeEden is a community fork built on **[Eden](https://git.eden-emu.dev/eden-emu/eden)** (GPL-3.0); all upstream credit remains with the Eden and yuzu projects. The rest of this README is Eden's upstream documentation.

---

## Compatibility

The emulator is capable of running most commercial games at full speed, provided you meet the necessary hardware requirements.

A list of supported games will be available in future. Please be patient.

Check out our [website](https://eden-emu.dev) for the latest news on exciting features, monthly progress reports, and more!

[![Packaging status](https://repology.org/badge/vertical-allrepos/eden-emulator.svg)](https://repology.org/project/eden-emulator/versions)

