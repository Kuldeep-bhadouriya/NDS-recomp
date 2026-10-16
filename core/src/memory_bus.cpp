#include "memory_bus.h"

MemRegions::MemRegions()
    : mainRam(4 * 1024 * 1024, 0),
      sharedWram(32 * 1024, 0),
      vram(656 * 1024, 0),
      oam(2048, 0) {}
