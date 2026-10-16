#pragma once
#include <string>
#include "emu_state.h"
#include "fast_forward.h"
#include "gpu.h"
#include "jit.h"
#include "memory_bus.h"
#include "snd.h"

// v0.3: JIT + GX + SPU wired (stubs, APIs frozen).
// State machine: STOPPED -> RUNNING <-> PAUSED, RUNNING <-> FAST_FORWARD.
class EmulatorCore {
 public:
  EmulatorCore();

  bool loadRom(const std::string& path, std::string* gamecodeOut);
  void unload();

  void setState(EmuState s);
  EmuState state() const { return state_; }

  void setFastForward(FFMode m);
  FastForwardGovernor& ff() { return ff_; }

  // One emulated frame. hint3DHeavy allows caller override (tests);
  // otherwise heaviness comes from GX renderer FIFO drain.
  // Returns false on fatal error (bad ROM, AP trip).
  bool stepFrame(bool hint3DHeavy = false);

  uint64_t frame() const { return frame_; }
  EmuSnapshot snapshotMeta() const;
  JitCache& jit() { return jit_; }
  GxRenderer& gpu() { return gpu_; }
  SpuMixer& spu() { return spu_; }

 private:
  EmuState state_ = EmuState::STOPPED;
  FastForwardGovernor ff_;
  MemRegions mem_;
  JitCache jit_;
  GxRenderer gpu_;
  SpuMixer spu_;
  CpuRegs arm9_{}, arm7_{};
  uint32_t arm9Pc_ = 0x02000800;  // Platinum entry until ROM boot parses header
  uint32_t cycles_ = 0;
  uint64_t frame_ = 0;
  std::string gamecode_;
};
