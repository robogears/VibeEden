# VibeEden v0.2.2

A small but important fix to the in-app updater.

## What's fixed
- **"Check for Updates" now works.** The update check sent its request to GitHub without a
  `User-Agent` header, which GitHub's API rejects with HTTP 403 - so every check failed with
  "Could not check for updates. Please check your internet connection and try again," even when you
  were online. VibeEden now sends a proper `User-Agent` on both the update check and the download.

> **One-time manual step for v0.2.1 users:** v0.2.1's checker is the broken one, so it can't fetch
> this update by itself. Download `Eden.AppImage` below once and replace your existing AppImage -
> from v0.2.2 on, every release updates itself automatically.

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

**Full Changelog**: https://github.com/robogears/VibeEden/compare/v0.2.1...v0.2.2
