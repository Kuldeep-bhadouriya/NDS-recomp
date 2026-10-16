#pragma once
#include <cstdint>

// Game-developer skill: explicit state machine, no string compares in hot loop.
enum class EmuState : uint8_t {
  STOPPED = 0,
  RUNNING = 1,
  PAUSED = 2,
  FAST_FORWARD = 3,  // uncapped / 2x / 4x handled by governor
};

enum class FFMode : uint8_t {
  OFF = 0,
  X2 = 2,
  X4 = 4,
  UNCAPPED = 255,
};
