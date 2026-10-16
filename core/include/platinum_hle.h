#pragma once
#include <cstdint>
#include <string>

// Platinum (CPUE) HLE hook table — Rev1 USA ONLY (sha1 0862ec35...).
// Capstone static pass 2026-10-14: DMA/GX/CARD candidates verified by
// disasm (prolog + reg-base + epilog); OS_ReadFile/SND/AP need Ghidra.
// Rule: enabled=false until Ghidra decompile confirms. Never enable on addr 0.
struct HleEntry {
  const char* nitroName;   // e.g. "MI_DmaCopy32"
  uint32_t arm9Addr;       // 0 = unresolved, filled after Ghidra import
  bool enabled;            // HLE handler active
  const char* evidence;    // capstone/Ghidra note
};

// Subset that matters for speed on Platinum. Full list in
// tools/ghidra/platinum_functions.csv.
// static (not inline var) for GCC6 host + NDK clang compatibility.
static HleEntry kPlatinumHleTable[] = {
    {"OS_ReadFile", 0x00000000, false, "unresolved: retail ARM9 stripped, needs Ghidra FNT/FAT xref"},
    {"MI_DmaCopy32", 0x021021A0, false, "candidate-high: PUSH@0x21021A0 DMA 0x040000B0 irq-guard POP@0x21021DC"},
    {"MI_DmaFill32", 0x021021E0, false, "candidate-medium: PUSH@0x21021E0 2nd DMA variant POP@0x2102244"},
    {"GX_SendFifo", 0x020B1084, false, "candidate-medium: PUSH10+0xC8 @0x20B1084 FIFO 0x04000400 x41 cluster"},
    {"SND_UpdateStream", 0x02383B00, false, "candidate-low: ARM7 SPU init/update cluster PUSH@0x2383B00 SPUCNT writes calls 0x238A09C; stream mixer needs ARM7 main-loop xref in Ghidra"},
    {"CARD_ReadSector", 0x020CD1C0, false, "candidate-medium: PUSH@0x20CD1C0 CARD 0x04100010 0x200-sector"},
    // Anti-piracy / overlay guard patches (NOP after verification):
    {"AP_OverlayCheck1", 0x00000000, false, "unresolved: in overlays (122, ov4/5 @0x21D0D80) needs Ghidra"},
    {"AP_OverlayCheck2", 0x00000000, false, "unresolved: second guard, verify both before patch"},
};

static constexpr uint32_t kPlatinumHleCount =
    sizeof(kPlatinumHleTable) / sizeof(kPlatinumHleTable[0]);

// Gamecode guard: only enable this table when header gamecode == "CPUE".
static inline bool IsPlatinumUSA(const std::string& gamecode) {
  return gamecode.size() == 4 && gamecode[0] == 'C' && gamecode[1] == 'P' &&
         gamecode[2] == 'U' && gamecode[3] == 'E';
}
