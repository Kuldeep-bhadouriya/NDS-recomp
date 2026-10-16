# Ghidra static pass — Platinum USA Rev1 (CPUE, sha1 0862ec35…) — 2026-10-14

## Verified by capstone (no Ghidra installed in this env)
- ARM9: fileoff 16384, RAM 0x02000000, size 1057784, entry 0x02000800 (clean ARM disasm).
- ARM7: fileoff 4233216, RAM 0x02380000, size 161788.
- Overlays: 122 total @fileoff 1074688; ov4 300672B + ov5 201632B @0x021D0D80 (field/battle).
- Retail ARM9 stripped: no NitroSDK/OS_ReadFile strings. Only sound-label CARD hits (SE_TRCARD).
- pokeplatinum confirms Rev1 target; SDK addrs must come from disasm, not strings.

## Candidates (see platinum_functions.csv, all enabled=false)
| Func | Addr | Evidence |
|---|---|---|
| MI_DmaCopy32 | 0x021021A0 | PUSH{r3-r7,lr}, DMA base 0x040000B0 pools @0x2102248/2D0, irq guard BL 0x21CDAB8/ACC, POP @0x21021DC |
| MI_DmaFill32 | 0x021021E0 | 2nd DMA variant same pattern, POP @0x2102244 — copy-vs-fill needs decompile split |
| GX_SendFifo | 0x020B1084 | PUSH10 + 0xC8 frame, FIFO 0x04000400 x41 cluster, calls 0x20B275C |
| CARD_ReadSector | 0x020CD1C0 | CARD 0x04100010 pools, 0x200-sector reads |
| OS_ReadFile / SND / AP x2 | 0 | need Ghidra: FNT/FAT xref, ARM7 0x2380000 pass, per-overlay import |

## To promote candidate -> confirmed (ghidra-reverse skill)
1. Ghidra import ARM9 @0x02000000 ARMv5t + overlays @0x021D0D80.
2. Decompile each addr, verify prototype (chan/src/dst/len) + callers in battle/map code.
3. Flip enabled=true one at a time, 60fps gate per game-developer skill.
4. AP: NEVER nop until both guards decompiled + save/overlay regression passes.
