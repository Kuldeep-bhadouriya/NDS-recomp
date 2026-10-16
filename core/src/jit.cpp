#include "jit.h"

namespace {
// ARM branch (B/BL) top byte 0xEA/0xEB; idle spin is cmp/bne pair to self.
bool isBranch(uint32_t w) { return (w >> 24) == 0xEA || (w >> 24) == 0xEB; }
}  // namespace

uint64_t JitCache::key(bool isArm9, uint32_t pc) {
  return (isArm9 ? 0x100000000ULL : 0ULL) | pc;
}

JitCache::JitCache(JitConfig cfg) : cfg_(cfg) { map_.reserve(4096); }

const JitBlock* JitCache::getBlock(bool isArm9, uint32_t pc,
                                   const uint8_t* mem, size_t memSize,
                                   uint32_t memBase) {
  uint64_t k = key(isArm9, pc);
  auto it = map_.find(k);
  if (it != map_.end()) return &it->second;
  JitBlock b;
  b.pc = pc;
  b.isArm9 = isArm9;
  // Scan max 32 ARM instrs (128B) for block end + hints. No alloc here.
  uint32_t off = (pc >= memBase) ? (pc - memBase) : 0;
  uint16_t n = 0;
  bool idle = false, gx = false;
  for (int i = 0; i < 32; ++i) {
    if (off + 4 > memSize) break;
    uint32_t w = (uint32_t)mem[off] | ((uint32_t)mem[off + 1] << 8) |
                 ((uint32_t)mem[off + 2] << 16) | ((uint32_t)mem[off + 3] << 24);
    ++n;
    off += 4;
    // GXFIFO store pattern flagged by caller too; detect literal 0x04000400-ish
    // low halfword traffic heuristically (full decode in codegen pass).
    if ((w & 0xFFFF0000) == 0x04000000) gx = true;
    if (isBranch(w)) break;  // end block at first branch (conservative v1)
    // idle: cmp rX,#0 / bne self seen at boot 0x2000808 (2x16-bit Thumb or ARM?)
    // v1 ARM check: cmp #0 followed by bne -8. Keep simple, codegen refines.
    if (i >= 1 && cfg_.idleSkip) {
      // placeholder: tight 2-block loop detection happens in execute()
    }
  }
  b.instrCount = n ? n : 1;
  b.isIdleLoop = idle;
  b.is3DHeavy = gx;
  auto res = map_.emplace(k, b);
  return &res.first->second;
}

uint32_t JitCache::execute(const JitBlock* b, uint32_t& cycles) {
  if (!b) return 0;
  // Idle skip: burn 4 cycles instead of spinning the A76 (FF battery win).
  if (b->isIdleLoop && cfg_.idleSkip) {
    cycles += 4;
    return b->pc;  // stay, yield timeslice at core level
  }
  cycles += (uint32_t)b->instrCount * 2;  // rough ARM9 CPI until dynarec
  return b->pc + (uint32_t)b->instrCount * 4;
}

void JitCache::invalidate(bool isArm9, uint32_t pc) {
  map_.erase(key(isArm9, pc));
}

void JitCache::invalidateAll() { map_.clear(); }
