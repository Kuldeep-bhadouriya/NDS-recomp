#include "fast_forward.h"

FastForwardGovernor::FastForwardGovernor() = default;

void FastForwardGovernor::setMode(FFMode mode) {
  mode_ = mode;
  frameCount_ = 0;
}

void FastForwardGovernor::setThermalLevel(int level) {
  thermal_ = level < 0 ? 0 : (level > 2 ? 2 : level);
}

bool FastForwardGovernor::shouldMuteAudio() const {
  return mode_ != FFMode::OFF;
}

bool FastForwardGovernor::shouldSkipDisplayCapture() const {
  return mode_ == FFMode::UNCAPPED || (mode_ == FFMode::X4 && thermal_ == 0);
}

uint32_t FastForwardGovernor::vsyncWaitMicros() const {
  if (mode_ == FFMode::OFF) return 16667;
  if (mode_ == FFMode::X2) return 8333;
  if (mode_ == FFMode::X4) return thermal_ >= 1 ? 8333 : 4166;
  return 0;  // UNCAPPED
}

bool FastForwardGovernor::onFrame(bool is3DHeavy) {
  ++frameCount_;
  if (mode_ != FFMode::UNCAPPED) return false;
  if (thermal_ >= 2) return false;  // severe: render everything, rely on vsync=0 only
  // SD678 guard: skip every 3rd heavy 3D frame to avoid Adreno 612 backlog.
  if (is3DHeavy && thermal_ >= 1) return (frameCount_ % 2) == 0;
  if (is3DHeavy) return (frameCount_ % 3) == 0;
  return false;
}

uint32_t FastForwardGovernor::effectiveMultiplier() const {
  if (mode_ == FFMode::UNCAPPED) {
    if (thermal_ == 1) return 4;
    if (thermal_ == 2) return 2;
    return 255;
  }
  return static_cast<uint32_t>(mode_);
}
