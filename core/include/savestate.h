#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Savestate format v1 (all little-endian):
//   0x00 magic[4]  = "PTST"
//   0x04 version   = 1 (u16)
//   0x06 gamecode[4] e.g. "CPUE" (Platinum USA)
//   0x0A slot     = u8
//   0x0B reserved = u8
//   0x0C frame    = u64 (emulated frame counter, RTC stays authoritative)
//   0x14 sectionCount = u16
//   then per section: id u16 + len u32 + payload + crc32 u32
//   file crc32 u32 at EOF
// Sections: 1=ARM9 regs, 2=ARM7 regs, 3=MAIN RAM ref, 4=WRAM, 5=VRAM,
//           6=IO/GPU, 7=SPU, 8=timers/DMA/IPC, 9=overlay/NitroFS cursor,
//           10=FLASH save mirror id + RTC delta
//
// Atomicity: write to slot.tmp then rename. Never overwrite in place.

struct SaveSection {
  uint16_t id = 0;
  std::vector<uint8_t> payload;
};

class SavestateManager {
 public:
  static constexpr uint32_t kMaxSlots = 8;
  static constexpr uint16_t kVersion = 1;

  explicit SavestateManager(std::string baseDir);

  // Serialize caller-provided sections. Returns file path on success.
  bool save(uint32_t slot, const std::string& gamecode, uint64_t frame,
            const std::vector<SaveSection>& sections, std::string* outPath);

  // Load and verify. Returns sections on success.
  bool load(uint32_t slot, std::string* gamecodeOut, uint64_t* frameOut,
            std::vector<SaveSection>* sectionsOut);

  static uint32_t crc32(const uint8_t* data, size_t len);

 private:
  std::string slotPath(uint32_t slot) const;
  std::string baseDir_;
};
