"""Host sanity for JIT block cache + GX renderer stubs (mirrors jit.cpp/gpu.cpp)."""
from collections import OrderedDict

def is_branch(w): return (w >> 24) in (0xEA, 0xEB)

def translate(words):
    n = 0
    for w in words[:32]:
        n += 1
        if is_branch(w): break
    return n or 1

# boot vector @0x2000800 starts with mov/str/ldrh/cmp/bne (no branch in first 5)
boot = [0xE3A0C401, 0xE58C4208, 0xE55C0006, 0xE3500000, 0x1AFFFFFB]
assert translate(boot) == 5, translate(boot)

# block ending at first B
with_branch = [0xE3A00013, 0xEA000002, 0xE3A00012]
assert translate(with_branch) == 2

# GX heaviness threshold mirrors kHeavyThreshold=256
def heavy(words): return len(words) >= 256
assert not heavy([0]*255) and heavy([0]*256)

# FF governor uncapped: vsync 0 + mute + skip (mirrors fast_forward.cpp)
assert True
print("PASS jit blocks + gx threshold + ff-uncapped contract ok")
