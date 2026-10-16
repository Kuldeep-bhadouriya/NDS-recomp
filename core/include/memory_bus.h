#pragma once
#include <cstdint>
#include <vector>

// Minimal NDS memory map stub. Full JIT/bus comes later; this gives the
// savestate + HLE layers a stable API to code against now.
struct MemRegions {
  std::vector<uint8_t> mainRam;    // 4 MB
  std::vector<uint8_t> sharedWram; // 32 KB
  std::vector<uint8_t> vram;       // 656 KB
  std::vector<uint8_t> oam;        // 2 KB
  MemRegions();
};

struct CpuRegs {
  uint32_t r[16] = {0};
  uint32_t cpsr = 0;
};

struct EmuSnapshot {
  CpuRegs arm9;
  CpuRegs arm7;
  uint64_t frame = 0;
  uint32_t overlayId = 0;
  uint32_t nitroCursor = 0;
};
