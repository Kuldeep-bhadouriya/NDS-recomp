# Platinum Fast (Path B)

Platinum-only fast runner for Android (SD678 / Android 16 target). NOT a general NDS emulator.

- Bring your own dump of Platinum USA Rev1 (`CPUE`). No ROM/BIOS/firmware bundled.
- Features: uncapped fast-forward w/ thermal guard, 8 save/load state slots (PTST v1).
- See `docs/architecture.md` + `tools/ghidra/README.md`.
- Build (Android): open in Android Studio, NDK r26+, CMake 3.22+. JNI lib `platinum_jni`.
- Host test: `python3 tests/host_sanity.py`
