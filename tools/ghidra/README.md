# Ghidra workflow for Platinum USA Rev1 (CPUE) — ghidra-reverse skill applied
# Skill steps: Project -> Import -> Analyze -> strings/xrefs -> decompile -> export

1. New Project `platinum_cpue_rev1`, Import `Pokemon - Platinum Version (USA) (Rev 1).nds`
   - Split ARM9 (offset 0x4000 in file, load 0x02000800) + ARM7 + overlays (see platinum_functions.csv).
   - Language: ARM:LE:32:v5t, base 0x02000000 for ARM9.
2. Auto-analyze with defaults. Record compiler guess (NitroSDK) + entry 0x02000800.
3. Strings to find: "MI_DmaCopy", "CARD_Read", "GX_", "OVERLAY", "AP_A". Follow xrefs to callers.
4. Decompile each kPlatinumHleTable entry, rename vars, add plate comments with addr.
5. Fill `arm9Addr` in core/include/platinum_hle.h. 0 = unresolved = HLE OFF (safe default).
6. Headless export (path varies — check your Ghidra install):
   analyzeHeadless /tmp/ghidra_proj Platinum -import <rom> -postScript ExportDecomp.py

Do NOT commit ROM, BIOS, or firmware. Only commit addresses + renamed prototypes.
