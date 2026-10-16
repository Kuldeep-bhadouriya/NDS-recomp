# Ghidra PyGhidra/headless stub: dump decompiled bodies for HLE table entries.
# Usage inside Ghidra Script Manager (python) or headless postScript.
# Replace getFunctionAt() addresses after step 4 of README.
try:
    from ghidra.app.decompiler import DecompInterface
    from ghidra.util.task import ConsoleTaskMonitor
except ImportError:
    pass

TARGETS = ["OS_ReadFile", "MI_DmaCopy32", "GX_SendFifo", "CARD_ReadSector"]

def dump(name, addr):
    print("== %s @ %s ==" % (name, hex(addr) if addr else "UNRESOLVED"))

for t in TARGETS:
    dump(t, 0)
print("Fill platinum_functions.csv with real addresses, then update platinum_hle.h")
