#pragma once
#include <cstdint>
#include <vector>

// Threaded GX renderer stub — Adreno 612 (OpenGL ES 3.0 / Vulkan-ready).
// v1: GXFIFO command queue + per-frame heavy flag for FF governor.
// Real rasterizer plugs into submitList()/renderFrame(); queue API frozen.
//
// NDS 3D quirks deferred to device pass: edge-marking, toon, w-buffer,
// capture regs. v1 classifies heaviness by FIFO word count + viewport writes.
struct GxCmd {
  uint32_t header = 0;
  uint32_t params[8] = {0};
  uint8_t paramCount = 0;
};

class GxRenderer {
 public:
  GxRenderer();

  // Called by JIT when ARM9 writes 0x04000400 (or HLE GX_SendFifo @0x20B1084).
  void pushFifo(uint32_t word);

  // Called at vblank. Returns true if frame was 3D-heavy (drives FF skip).
  bool renderFrame(bool ffSkipAllowed);

  void reset();
  uint32_t pendingWords() const { return (uint32_t)queue_.size(); }
  uint64_t frames() const { return frames_; }
  uint64_t heavyFrames() const { return heavy_; }

 private:
  std::vector<uint32_t> queue_;  // pooled, reserve() once (no per-frame alloc)
  uint64_t frames_ = 0;
  uint64_t heavy_ = 0;
  static constexpr uint32_t kHeavyThreshold = 256;  // tuned for Platinum overworld
};
