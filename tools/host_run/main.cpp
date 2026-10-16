// Host demo runner: proves core boots ROM header, steps frames, FF + states work.
// NOT gameplay — CPU/GPU/SPU are stubs. Real dynarec/rasterizer still TODO.
#include <cstdio>
#include <string>
#include <vector>
#include "emulator_core.h"
#include "savestate.h"

int main(int argc, char** argv) {
  std::string rom = (argc > 1) ? argv[1]
                               : "D:/NDS emu/Pokemon - Platinum Version (USA) (Rev 1).nds";
  std::string dir = (argc > 2) ? argv[2] : "C:/Users/Kuldeep/AppData/Local/Temp/opencode";
  EmulatorCore core;
  std::string gc;
  if (!core.loadRom(rom, &gc)) {
    printf("LOAD_FAIL %s\n", rom.c_str());
    return 1;
  }
  printf("LOAD_OK gamecode=%s\n", gc.c_str());

  core.setFastForward(FFMode::UNCAPPED);
  for (int i = 0; i < 600; ++i) {
    // alternate heavy hint to exercise GX path (overworld-ish)
    core.stepFrame((i % 3) == 0);
    if (i % 200 == 0) core.gpu().pushFifo(0xDEADBEEF);
  }
  printf("FRAMES=%llu jit_blocks=%zu gpu_frames=%llu gpu_heavy=%llu spu_updates=%llu ff_eff=%u\n",
         (unsigned long long)core.frame(), core.jit().cachedBlocks(),
         (unsigned long long)core.gpu().frames(),
         (unsigned long long)core.gpu().heavyFrames(),
         (unsigned long long)core.spu().updates(),
         core.ff().effectiveMultiplier());

  SavestateManager sm(dir);
  auto meta = core.snapshotMeta();
  std::vector<SaveSection> secs;
  secs.push_back({1, std::vector<uint8_t>((uint8_t*)&meta.arm9, (uint8_t*)&meta.arm9 + sizeof(meta.arm9))});
  secs.push_back({2, std::vector<uint8_t>((uint8_t*)&meta.arm7, (uint8_t*)&meta.arm7 + sizeof(meta.arm7))});
  std::string out;
  bool sok = sm.save(0, gc.size() == 4 ? gc : "CPUE", meta.frame, secs, &out);
  printf("SAVE slot0 %s path=%s\n", sok ? "OK" : "FAIL", out.c_str());
  std::string lgc;
  uint64_t lfr = 0;
  std::vector<SaveSection> lsecs;
  bool lok = sm.load(0, &lgc, &lfr, &lsecs);
  printf("LOAD slot0 %s gamecode=%s frame=%llu sections=%zu\n", lok ? "OK" : "FAIL",
         lgc.c_str(), (unsigned long long)lfr, lsecs.size());
  return (sok && lok) ? 0 : 2;
}
