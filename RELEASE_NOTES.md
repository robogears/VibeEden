# VibeEden v0.2.4

A maintenance release with **no functional changes** - it exists so a v0.2.3 install can exercise the
in-app self-updater end to end (check -> download -> SHA-256 verify -> relaunch), now that the
updater itself works (User-Agent fix in v0.2.2, CA-certificate fix in v0.2.3). If you are on v0.2.3,
the status-bar **Check for Updates** button should find this release and install it automatically.

## Install
- **Steam Deck / Linux**: download `Eden.AppImage`, mark it executable (`chmod +x Eden.AppImage`),
  and add it to Steam as a non-Steam game with **Steam Input** enabled. Built for glibc 2.38+
  (current SteamOS).
- VibeEden updates itself - use the status-bar **Check for Updates** button, or the prompt on launch.

## Requirements
- A current SteamOS / Linux (glibc 2.38+) with working Vulkan drivers.
- Steam Input enabled for the controllers you want VibeEden to use.

---

**Full Changelog**: https://github.com/robogears/VibeEden/compare/v0.2.3...v0.2.4
