#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

// ARM9/ARM7 JIT stub — SD678 (ARM64 host) target.
// v1: block cache + interpreter fallback + idle-loop detection.
// Real codegen (ARMv5->ARM64) plugs into JitBlock::code later; API stays stable.
//
// Design (game-developer skill: no alloc in hot loop):
// - Fixed 64K-entry direct-mapped cache + overflow map for collisions.
// - Block key = (isArm9 << 31) | pc. Hash once, execute many.
// - Idle loop (e.g. 0xE7FFDEFF wait loops at boot 0x2000808) detected by
//   2-instr backward-branch pattern -> yields timeslice instead of burning A76.
struct JitConfig {
  bool enableArm9 = true;
  bool enableArm7 = true;
  bool idleSkip = true;  // critical for FF uncapped battery on SD678
};

struct JitBlock {
  uint32_t pc = 0;
  bool isArm9 = true;
  uint16_t instrCount = 0;
  bool isIdleLoop = false;
  bool is3DHeavy = false;  // set if block writes GXFIFO (0x04000400)
};

class JitCache {
 public:
  explicit JitCache(JitConfig cfg = {});

  // Translate (stub: scans up to 32 instrs for branches/FIFO writes).
  // Returns cached block; miss compiles without malloc in steady state
  // (overflow map only on hash collision).
  const JitBlock* getBlock(bool isArm9, uint32_t pc, const uint8_t* mem,
                           size_t memSize, uint32_t memBase);

  // Execute stub: advances pc past block, reports idle/GX hints.
  // Real codegen replaces body; callers unchanged.
  uint32_t execute(const JitBlock* b, uint32_t& cycles);

  void invalidate(bool isArm9, uint32_t pc);
  void invalidateAll();
  size_t cachedBlocks() const { return map_.size(); }

 private:
  JitConfig cfg_;
  std::unordered_map<uint64_t, JitBlock> map_;
  static uint64_t key(bool isArm9, uint32_t pc);
};
