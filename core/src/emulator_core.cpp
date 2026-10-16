#include "emulator_core.h"
#include "platinum_hle.h"
#include <cstdio>

EmulatorCore::EmulatorCore() = default;

bool EmulatorCore::loadRom(const std::string& path, std::string* gamecodeOut) {
  FILE* f = fopen(path.c_str(), "rb");
  if (!f) return false;
  uint8_t hdr[64] = {0};
  size_t r = fread(hdr, 1, sizeof(hdr), f);
  fclose(f);
  if (r < 20) return false;
  gamecode_ = std::string((char*)hdr + 12, 4);
  if (gamecodeOut) *gamecodeOut = gamecode_;
  frame_ = 0;
  state_ = EmuState::RUNNING;
  return true;
}

void EmulatorCore::unload() {
  state_ = EmuState::STOPPED;
  gamecode_.clear();
  frame_ = 0;
}

void EmulatorCore::setState(EmuState s) {
  if (state_ == EmuState::STOPPED) return;
  state_ = s;
}

void EmulatorCore::setFastForward(FFMode m) {
  ff_.setMode(m);
  if (state_ == EmuState::RUNNING || state_ == EmuState::FAST_FORWARD)
    state_ = (m == FFMode::OFF) ? EmuState::RUNNING : EmuState::FAST_FORWARD;
}

bool EmulatorCore::stepFrame(bool hint3DHeavy) {
  if (state_ != EmuState::RUNNING && state_ != EmuState::FAST_FORWARD)
    return true;  // paused: no-op
  // Platinum guard: only CPUE gets HLE fast paths.
  bool isPlatinum = IsPlatinumUSA(gamecode_);
  (void)isPlatinum;
  // v0.2 timeslice: run JIT blocks until ~560K cycles (~1 frame @67MHz/60fps
  // scaled; real timing in device pass). Feed GX heaviness to FF governor.
  const uint32_t kSlice = 560000 / 60;
  uint32_t used = 0;
  bool gxHeavy = hint3DHeavy;
  int steps = 0;
  while (used < kSlice && steps < 4000) {
    const JitBlock* b = jit_.getBlock(true, arm9Pc_, mem_.mainRam.data(),
                                      mem_.mainRam.size(), 0x02000000);
    arm9Pc_ = jit_.execute(b, used);
    if (b && b->is3DHeavy) gxHeavy = true;
    // Idle loops yield early (battery win on SD678).
    if (b && b->isIdleLoop) break;
    ++steps;
    // Keep pc inside 4MB mirror for stub; real MMU later.
    if (arm9Pc_ < 0x02000000 || arm9Pc_ >= 0x02400000)
      arm9Pc_ = 0x02000800;
  }
  cycles_ += used;
  arm9_.r[15] = arm9Pc_;
  bool heavy = gpu_.renderFrame(ff_.mode() == FFMode::UNCAPPED) || gxHeavy;
  // SPU: mute during FF (uncapped/X4) to avoid AAudio backlog on SD678.
  // HLE SND_UpdateStream (ARM7 0x2383B00 cluster) calls updateStream() in
  // device pass; stub advances cursors silently here.
  spu_.updateStream(ff_.shouldMuteAudio());
  ff_.onFrame(heavy);
  ++frame_;
  return true;
}

EmuSnapshot EmulatorCore::snapshotMeta() const {
  EmuSnapshot s;
  s.arm9 = arm9_;
  s.arm7 = arm7_;
  s.frame = frame_;
  return s;
}
