# VibeEden v0.2.3

The companion fix to v0.2.2 - the in-app updater now works inside the AppImage on the Steam Deck.

## What's fixed
- **"Check for Updates" connects on SteamOS now.** v0.2.2 fixed the missing `User-Agent`, but the
  AppImage bundles its own copy of OpenSSL whose CA-certificate directory is hard-set to a path that
  does not exist on SteamOS - so the HTTPS connection to GitHub could not be verified and the check
  still failed with "could not check for updates." VibeEden now points its updater at the system CA
  bundle (`/etc/ssl/certs/...`, with fallbacks), so both the update check and the download verify
  correctly inside the AppImage.

> **One-time manual step for v0.2.1 / v0.2.2 users:** those builds can't fetch this update on their
> own (their checker is the broken one). Download `Eden.AppImage` below once and replace your
> existing AppImage - from v0.2.3 on, every release updates itself automatically.

## Install
- **Steam Deck / Linux**: download `Eden.AppImage`, mark it executable (`chmod +x Eden.AppImage`),
  and add it to Steam as a non-Steam game with **Steam Input** enabled. Built for glibc 2.38+
  (current SteamOS).
- From here on VibeEden updates itself - use the status-bar **Check for Updates** button, or the
  prompt on launch.

## Requirements
- A current SteamOS / Linux (glibc 2.38+) with working Vulkan drivers.
- Steam Input enabled for the controllers you want VibeEden to use.

---

**Full Changelog**: https://github.com/robogears/VibeEden/compare/v0.2.2...v0.2.3
