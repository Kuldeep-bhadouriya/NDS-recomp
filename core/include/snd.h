#pragma once
#include <cstdint>
#include <vector>

// ARM7 SPU mixer stub — NDS 16ch x ADPCM/PSG, 32768Hz.
// v1: stream-update hook + FF mute (the uncapped-FF requirement).
// Real resampler plugs into mixFrame(); control API frozen.
//
// FF rule (SD678): muted during UNCAPPED/X4 to avoid AAudio backlog;
// X2 keeps audio (pitch preserved, frames drop instead).
class SpuMixer {
 public:
  SpuMixer();

  // Called per emulated frame from ARM7 timeslice (or HLE SND_UpdateStream).
  // If muted, advances cursors without writing samples.
  void updateStream(bool muted);

  // Platform pulls mixed frames; stub returns silence when muted.
  uint32_t pullSamples(int16_t* out, uint32_t maxFrames);

  void reset();
  uint64_t updates() const { return updates_; }
  bool muted() const { return muted_; }

 private:
  std::vector<int16_t> pool_;  // reused buffer, no per-frame alloc
  uint64_t updates_ = 0;
  bool muted_ = false;
  static constexpr uint32_t kRate = 32768;
  static constexpr uint32_t kFrameSamples = 546;  // 32768/60
};
