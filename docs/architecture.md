# Platinum Fast — Path B (Platinum-only)

Fork of melonDS core idea, stripped to CPUE Rev1 only.

## Why Path B wins on SD678
- No general-case overhead. Only CPUE overlay IDs, only NitroSDK versions Platinum uses.
- HLE fast paths for MI_Dma + CARD reads (biggest stutter source).
- FF uncapped: vsync=0, SPU muted, display-capture skipped, every-3rd heavy 3D frame skipped w/ thermal guard.
- Savestates v1: PTST container, CRC32 per section + file, atomic rename, 8 slots. Pause at vblank before dump (caller enforces).

## Layout
- `core/` portable C++17, no Android deps. Builds as host static lib for tests + device static lib for JNI.
- `app/` thin Kotlin shell, SAF ROM picker (user provides own dump), no bundled ROM/BIOS.
- `tools/ghidra/` RE kit per ghidra-reverse skill.

## Bring-up order
1. Fill platinum_functions.csv from Ghidra, update platinum_hle.h addrs, enable HLE one by one.
2. Land ARM JIT + threaded GX renderer (stubs now).
3. Full RAM/VRAM sections in JNI saveState (regs only now — see TODO in jni_bridge.cpp).
4. Thermal listener -> setThermalLevel() for uncapped sustain.

## Constraints (game-developer skill)
- 60 FPS validation before enabling new HLE.
- Object-pool frame/GXFIFO buffers, no alloc in stepFrame().
- Async overlay loads, delta-time UI, state machine EmuState (no string compares).
