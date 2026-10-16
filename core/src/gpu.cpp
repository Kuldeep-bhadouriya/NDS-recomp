#include "gpu.h"

GxRenderer::GxRenderer() { queue_.reserve(4096); }

void GxRenderer::pushFifo(uint32_t word) {
  if (queue_.size() < queue_.capacity()) queue_.push_back(word);
}

bool GxRenderer::renderFrame(bool ffSkipAllowed) {
  ++frames_;
  // Heavy = FIFO burst over threshold (Platinum overworld/battle) OR
  // sustained queue backlog (Adreno 612 backlog guard).
  bool heavy = queue_.size() >= kHeavyThreshold;
  if (heavy) ++heavy_;
  queue_.clear();  // keep capacity (no realloc — game-developer pooling rule)
  // Caller (FF governor) decides skip next frame; we only report.
  (void)ffSkipAllowed;
  return heavy;
}

void GxRenderer::reset() {
  queue_.clear();
  frames_ = heavy_ = 0;
}
