# GhostCity native integration

Base repository: https://github.com/Testando0/re3-android-port-evolved.git
Branch: `ghostcity-staunton`

Chosen principal city: **Staunton Island / LEVEL_COMMERCIAL**, because it is the downtown/high-rise area of Liberty City and best fits the reference composition.

The repository is a native C++/RenderWare re3 Android port. It uses an Android Gradle launcher at `android/launcher`, CMake, NDK 27.2.12479018 and the `vendor/librw` submodule. The clone must include all git submodules.

Baseline Android debug build succeeded after installing NDK/CMake and running `:app:assembleDebug`. The generated APK is under `android/launcher/app/build/outputs/apk/debug/`.

Implemented in this branch so far:
- `src/core/GhostCity.h/.cpp`: native GhostCity plaza bootstrap for `LEVEL_COMMERCIAL`, reusing native GTA3 tree, lamp, bollard and park props; symmetry around a central plaza; native magenta corona heart points; cleanup support.
- `src/core/Game.cpp`: GhostCity initialization after `ReloadIPLs`, per-frame corona processing, shutdown cleanup.

Important limitations still to solve before release:
- Coordinates and prop selection need in-game visual validation on Staunton Island.
- A real heart mesh/sign and authored plaza assets are still needed for a close match to the reference.
- re3 has no multiplayer/P2P implementation in this fork. A native P2P transport and remote-avatar synchronization must be implemented and tested; direct UDP is not enough for arbitrary mobile NAT without a signaling/relay path.
- No commit/release should be made until the modified APK builds, installs, launches, the plaza is visible, and P2P tests pass.
