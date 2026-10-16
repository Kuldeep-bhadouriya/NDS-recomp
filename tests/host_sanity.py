"""Host-side sanity for PTST savestate layout (mirrors savestate.cpp).
Run: python3 tests/host_sanity.py  (no ROM/BIOS needed)
"""
import struct, zlib, os, tempfile

def crc32(b): return zlib.crc32(b) & 0xFFFFFFFF

def build(slot=0, gamecode=b"CPUE", frame=1234):
    b = bytearray()
    b += b"PTST"; b += struct.pack("<H", 1); b += gamecode
    b += struct.pack("BB", slot, 0); b += struct.pack("<Q", frame)
    secs = [(1, b"ARM9" * 4), (2, b"ARM7" * 4)]
    b += struct.pack("<H", len(secs))
    for sid, p in secs:
        b += struct.pack("<H", sid) + struct.pack("<I", len(p)) + p + struct.pack("<I", crc32(p))
    b += struct.pack("<I", crc32(bytes(b)))
    return bytes(b)

def parse(buf):
    assert buf[:4] == b"PTST", "bad magic"
    assert struct.unpack("<H", buf[4:6])[0] == 1, "bad version"
    assert buf[6:10] == b"CPUE", "not Platinum USA"
    assert struct.unpack("<I", buf[-4:])[0] == crc32(buf[:-4]), "file crc mismatch"
    print("PASS magic=PTST version=1 gamecode=CPUE crc=ok len=%d" % len(buf))

if __name__ == "__main__":
    buf = build()
    d = tempfile.mkdtemp()
    p = os.path.join(d, "platinum_slot0.ptst")
    open(p, "wb").write(buf)
    parse(open(p, "rb").read())
    print("slot file:", p)
