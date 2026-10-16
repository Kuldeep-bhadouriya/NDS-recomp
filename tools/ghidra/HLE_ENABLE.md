# HLE enable gates — one at a time, 60fps validation each (game-developer skill)

Rule: `enabled=false` until ALL boxes ticked. Never enable on addr 0.

## Per-function checklist
- [ ] Ghidra decompile matches prototype (args, regs, returns)
- [ ] Callers confirmed in Platinum battle/map/overlay code (xrefs)
- [ ] `IsPlatinumUSA(gamecode)` guard passes (CPUE Rev1 sha1 0862ec35…)
- [ ] 60fps gate: 5-min overworld + battle loop, no regress on SD678
- [ ] Savestate round-trip after enable (save → load → same frame+1)

## Order (speed impact first, risk last)
1. MI_DmaCopy32 @0x021021A0 (candidate-high) — battle/map DMA
2. CARD_ReadSector @0x020CD1C0 — overlay stutter
3. GX_SendFifo @0x020B1084 — 3D funnel (needs rasterizer first)
4. MI_DmaFill32 @0x021021E0 — after copy confirmed (split copy vs fill)
5. SND_UpdateStream @0x02383B00 ARM7 — after mixer lands
6. OS_ReadFile — after FNT/FAT xref
7. AP checks — LAST, both guards + save/overlay regression, never blind NOP
