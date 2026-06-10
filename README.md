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

<p align="center">
    </a>
    <a href="https://discord.gg/HstXbPch7X">
        <img src="https://img.shields.io/discord/1367654015269339267?color=5865F2&label=Eden&logo=discord&logoColor=white"
            alt="Discord">
    </a>
    <a href="https://stt.gg/qKgFEAbH">
        <img src="https://img.shields.io/revolt/invite/qKgFEAbH?color=d61f3a&label=Stoat"
            alt="Stoat">
    </a>
</p>

<p align="center">
  <a href="#compatibility">Compatibility</a> |
  <a href="#development">Development</a> |
  <a href="#building">Building</a> |
  <a href="#download">Download</a> |
  <a href="#support">Support</a> |
  <a href="#license">License</a>
</p>

---

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

## Development

Most of the development happens on our Git server. It is also where [our central repository](https://git.eden-emu.dev/eden-emu/eden) is hosted. For development discussions, please join us on [Discord](https://discord.gg/HstXbPch7X) or [Stoat](https://stt.gg/qKgFEAbH).
You can also follow us on [X (Twitter)](https://nitter.poast.org/edenemuofficial) for updates and announcements.

If you would like to contribute, we are open to new developers and pull requests. Please ensure that your work is of a high standard and properly documented. You can also contact any of the developers on Discord or Stoat to learn more about the current state of the emulator.

See the [sign-up instructions](docs/SIGNUP.md) for information on registration.

Alternatively, if you wish to add translations, go to the [Eden project on Transifex](https://app.transifex.com/edenemu/eden-emulator) and review [the translations README](./dist/languages).

## Documentation

We have a user manual! See our [User Handbook](./docs/user/README.md).

## Building

See the [General Build Guide](docs/Build.md)

For information on provided development tooling, see the [Tools directory](./tools)

## Download

You can download the latest releases from [here](https://git.eden-emu.dev/eden-emu/eden/releases).

Save us some bandwidth! We have [mirrors available](./docs/user/ThirdParty.md#mirrors) as well.

## Support

If you enjoy the project and would like to support us financially, please check out our developers' [donation pages](https://eden-emu.dev/donations)!

Any donations received will go towards things such as:
* Switch consoles to explore and reverse-engineer the hardware
* Switch games for testing, reverse-engineering, and implementing new features
* Web hosting and infrastructure setup
* Additional hardware (e.g. GPUs as needed to improve rendering support, other peripherals to add support for, etc.)
* CI Infrastructure

If you would prefer to support us in a different way, please join our [Discord](https://discord.gg/HstXbPch7X) and talk to Camille or any of our other developers.

## License

Eden is licensed under the GPLv3 (or any later version). Refer to the [LICENSE.txt](https://git.eden-emu.dev/eden-emu/eden/src/branch/master/LICENSE.txt) file.
