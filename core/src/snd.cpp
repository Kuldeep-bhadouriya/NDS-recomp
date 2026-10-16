#include "snd.h"
#include <cstring>

SpuMixer::SpuMixer() { pool_.assign(kFrameSamples * 2, 0); }

void SpuMixer::updateStream(bool muted) {
  muted_ = muted;
  ++updates_;
  // Stub: advance 16ch cursors here in device pass; keep pool silent.
  if (muted) {
    memset(pool_.data(), 0, pool_.size() * sizeof(int16_t));
  }
}

uint32_t SpuMixer::pullSamples(int16_t* out, uint32_t maxFrames) {
  uint32_t n = maxFrames < kFrameSamples ? maxFrames : kFrameSamples;
  if (!out) return n;
  if (muted_) {
    memset(out, 0, n * 2 * sizeof(int16_t));
  } else {
    // Stub silence until resampler lands (avoids garbage on Oboe).
    memset(out, 0, n * 2 * sizeof(int16_t));
  }
  return n;
}

void SpuMixer::reset() {
  updates_ = 0;
  muted_ = false;
  memset(pool_.data(), 0, pool_.size() * sizeof(int16_t));
}
