# Build the APK — 3 ways (no Android Studio required if you use A or B)

## A. Cloud build (easiest, nothing to install)
1. `git init` in `platinum-fast`, push to a GitHub repo (keep it private —
   it contains no ROM, only your RE notes + addresses).
2. The workflow `.github/workflows/build-apk.yml` runs on push:
   JDK 17 + SDK 36 + NDK r26 + Gradle 8.7 → `:app:assembleDebug`.
3. Download `platinum-fast-debug` from the run's Artifacts →
   `app-debug.apk`, sideload to your SD678 phone.

## B. Headless local build (Windows script, no Studio)
1. In PowerShell: `.\tools\build-apk-local.ps1`
   (fetches Temurin JDK 17 + cmdline-tools + SDK 36 + NDK r26 + Gradle 8.7,
   ~2GB first run, cached after; your system Java 24 is bypassed, not used).
2. APK lands at `app\build\outputs\apk\debug\app-debug.apk`.

## C. Android Studio (classic)

1. Install Android Studio (Hedgehog+), SDK Platform 36, NDK r26+, CMake 3.22+.
2. Copy `local.properties.template` -> `local.properties`, set `sdk.dir`.
3. Open `D:\NDS emu\platinum-fast` as the Gradle project (settings.gradle at root).
4. `Build > Make Project`, then `Run 'app'` on your SD678 / Android 16 device.
   Or CLI: `gradlew :app:assembleDebug` → `app/build/outputs/apk/debug/app-debug.apk`
5. On device: open app → Pick ROM → choose your own Platinum CPUE dump
   (sha1 `0862ec35…`; file is copied to app-private `rom.nds`, never bundled).
   Tap S0-S3 to save, long-press to load. FF cycles OFF→2X→4X→UNCAPPED.

## What the shell does today (v0.3-shell)
- SAF picker + private copy, `loadRom` header check (CPUE), frame thread
  (~60Hz paced, uncapped when FF), thermal listener → `setThermalLevel`,
  touch view mapping bottom-half → 256x192, 4 state slots in UI (8 in core).
- JNI (`jni_bridge.cpp`) matches `CoreBridge.kt` 1:1 — verified by inspection;
  native compile of the same sources passes via host g++ (see root README).

## Still stub (device pass next)
- `stepFrame` runs JIT/GPU/SPU stubs — boots header, does NOT play the game.
  Real ARM interpreter → dynarec, MMU/overlay loader, GLES renderer into
  `EmuView`, and Oboe audio are the next milestones before playability.
