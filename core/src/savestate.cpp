#include "savestate.h"
#include <cstdio>
#include <cstring>

namespace {
void putU16(std::vector<uint8_t>& b, uint16_t v) {
  b.push_back(static_cast<uint8_t>(v & 0xFF));
  b.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}
void putU32(std::vector<uint8_t>& b, uint32_t v) {
  for (int i = 0; i < 4; ++i) b.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
}
void putU64(std::vector<uint8_t>& b, uint64_t v) {
  for (int i = 0; i < 8; ++i) b.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
}
uint16_t getU16(const uint8_t* p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
uint32_t getU32(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}
uint64_t getU64(const uint8_t* p) {
  uint64_t v = 0;
  for (int i = 0; i < 8; ++i) v |= (uint64_t)p[i] << (8 * i);
  return v;
}
}  // namespace

SavestateManager::SavestateManager(std::string baseDir) : baseDir_(baseDir) {}

std::string SavestateManager::slotPath(uint32_t slot) const {
  char buf[256];
  snprintf(buf, sizeof(buf), "%s/platinum_slot%u.ptst", baseDir_.c_str(), slot);
  return std::string(buf);
}

uint32_t SavestateManager::crc32(const uint8_t* data, size_t len) {
  static uint32_t table[256];
  static bool init = false;
  if (!init) {
    for (uint32_t i = 0; i < 256; ++i) {
      uint32_t c = i;
      for (int k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
      table[i] = c;
    }
    init = true;
  }
  uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
  return c ^ 0xFFFFFFFFu;
}

bool SavestateManager::save(uint32_t slot, const std::string& gamecode,
                            uint64_t frame,
                            const std::vector<SaveSection>& sections,
                            std::string* outPath) {
  if (slot >= kMaxSlots || gamecode.size() != 4) return false;
  std::vector<uint8_t> b;
  b.insert(b.end(), {'P', 'T', 'S', 'T'});
  putU16(b, kVersion);
  b.insert(b.end(), gamecode.begin(), gamecode.end());
  b.push_back(static_cast<uint8_t>(slot));
  b.push_back(0);
  putU64(b, frame);
  putU16(b, static_cast<uint16_t>(sections.size()));
  for (const auto& s : sections) {
    putU16(b, s.id);
    putU32(b, static_cast<uint32_t>(s.payload.size()));
    b.insert(b.end(), s.payload.begin(), s.payload.end());
    putU32(b, crc32(s.payload.data(), s.payload.size()));
  }
  putU32(b, crc32(b.data(), b.size()));
  std::string tmp = slotPath(slot) + ".tmp";
  std::string dst = slotPath(slot);
  FILE* f = fopen(tmp.c_str(), "wb");
  if (!f) return false;
  size_t w = fwrite(b.data(), 1, b.size(), f);
  fclose(f);
  if (w != b.size()) return false;
  if (rename(tmp.c_str(), dst.c_str()) != 0) return false;
  if (outPath) *outPath = dst;
  return true;
}

bool SavestateManager::load(uint32_t slot, std::string* gamecodeOut,
                            uint64_t* frameOut,
                            std::vector<SaveSection>* sectionsOut) {
  if (slot >= kMaxSlots) return false;
  std::string path = slotPath(slot);
  FILE* f = fopen(path.c_str(), "rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (n < 22) { fclose(f); return false; }
  std::vector<uint8_t> b((size_t)n);
  size_t r = fread(b.data(), 1, b.size(), f);
  fclose(f);
  if (r != b.size()) return false;
  uint32_t fileCrc = getU32(b.data() + b.size() - 4);
  if (crc32(b.data(), b.size() - 4) != fileCrc) return false;
  if (memcmp(b.data(), "PTST", 4) != 0) return false;
  if (getU16(b.data() + 4) != kVersion) return false;
  std::string gc((char*)b.data() + 6, 4);
  if (b[10] != slot) return false;
  uint64_t frame = getU64(b.data() + 12);
  uint16_t count = getU16(b.data() + 20);
  size_t off = 22;
  std::vector<SaveSection> out;
  for (int i = 0; i < count; ++i) {
    if (off + 6 > b.size() - 4) return false;
    uint16_t id = getU16(b.data() + off); off += 2;
    uint32_t len = getU32(b.data() + off); off += 4;
    if (off + len + 4 > b.size() - 4) return false;
    SaveSection s;
    s.id = id;
    s.payload.assign(b.begin() + off, b.begin() + off + len);
    off += len;
    uint32_t sc = getU32(b.data() + off); off += 4;
    if (crc32(s.payload.data(), s.payload.size()) != sc) return false;
    out.push_back(std::move(s));
  }
  if (gamecodeOut) *gamecodeOut = gc;
  if (frameOut) *frameOut = frame;
  if (sectionsOut) *sectionsOut = std::move(out);
  return true;
}
