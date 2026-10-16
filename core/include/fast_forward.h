#pragma once
#include <cstdint>
#include "emu_state.h"

// Fast-forward governor tuned for SD678 (2x A76 + 6x A55, Adreno 612).
// Uncapped = no vsync wait, SPU muted, display-capture skipped.
// Thermal guard steps UNCAPPED -> X4 -> X2 when throttling.
class FastForwardGovernor {
 public:
  FastForwardGovernor();

  void setMode(FFMode mode);
  FFMode mode() const { return mode_; }

  // Called once per emulated frame. Returns true if this frame's 3D
  // layer may be skipped to hold speed (FF only).
  bool onFrame(bool is3DHeavy);

  // Audio/video sync hints for the platform layer.
  bool shouldMuteAudio() const;
  bool shouldSkipDisplayCapture() const;
  uint32_t vsyncWaitMicros() const;  // 0 = uncapped

  // Thermal callback from Android (0=normal, 1=moderate, 2=severe).
  void setThermalLevel(int level);

  // Effective speed multiplier for UI display (255 = uncapped).
  uint32_t effectiveMultiplier() const;

 private:
  FFMode mode_ = FFMode::OFF;
  int thermal_ = 0;
  uint32_t frameCount_ = 0;
};
