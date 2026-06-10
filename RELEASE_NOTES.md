# VibeEden v0.2.1

VibeEden is a Steam-Deck-focused fork of the Eden Switch emulator. The whole point is making controllers **just work** under SteamOS: Steam decides the controller order, VibeEden follows it and binds each pad automatically as a Pro Controller, a mid-game disconnect is a no-op, and the app keeps itself updated. See the [README](https://github.com/robogears/VibeEden#vibeeden--steam-deck-controller-rework) for the full rundown.

## What's in this release

### Controllers (the headline feature)
- Connect a controller and it is auto-bound to the next player in **Steam's order**, as a Pro Controller, with zero setup.
- Disconnect mid-game and **nothing happens** - no pause, no popup; reconnect and you keep playing on the same slot.
- Binds only Steam virtual gamepads; a clean, dark, auto-resuming prompt only appears when a game genuinely needs more controllers.

### Security & stability hardening
- The game-file loaders (NSO / NRO / PFS / KIP / RomFS / NPDM and the VFS layer) are now bounds-checked against malformed or malicious files - a crafted dump can no longer trigger the out-of-bounds reads/writes or giant allocations that were possible before. (From a full security audit; protects anyone who opens an untrusted file.)
- Hardened the multiplayer room handler against a zero-length-packet crash.

### Updater
- The in-app updater verifies the download's **SHA-256** against GitHub's per-asset digest before the file is ever made executable, downloads over HTTPS only, and won't leave you with a closed app if an update can't be applied.
- **"Check for Updates" is now a dedicated button in the status bar** (next to the renderer / dock / FSR / volume indicators) - moved out of the Help menu and the controller settings page.

---

## Install
- **Steam Deck / Linux**: download `Eden.AppImage`, mark it executable (`chmod +x Eden.AppImage`), and add it to Steam as a non-Steam game with **Steam Input** enabled. Built for glibc 2.38+ (current SteamOS).
- From here on VibeEden updates itself - use the status-bar **Check for Updates** button, or the prompt on launch.

## Requirements
- A current SteamOS / Linux (glibc 2.38+) with working Vulkan drivers.
- Steam Input enabled for the controllers you want VibeEden to use.

---

**Full Changelog**: https://github.com/robogears/VibeEden/compare/v0.1.4...v0.2.1
