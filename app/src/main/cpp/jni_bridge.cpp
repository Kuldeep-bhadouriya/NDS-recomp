#include <jni.h>
#include <string>
#include "emulator_core.h"
#include "savestate.h"

static EmulatorCore g_core;
static std::string g_baseDir = "/data/data/com.platinumfast/files";

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_platinumfast_CoreBridge_loadRom(JNIEnv* env, jobject, jstring jpath) {
  const char* p = env->GetStringUTFChars(jpath, nullptr);
  std::string gc;
  bool ok = g_core.loadRom(p, &gc);
  env->ReleaseStringUTFChars(jpath, p);
  return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_platinumfast_CoreBridge_unload(JNIEnv*, jobject) {
  g_core.unload();
}

JNIEXPORT void JNICALL
Java_com_platinumfast_CoreBridge_setFastForward(JNIEnv*, jobject, jint mode) {
  FFMode m = FFMode::OFF;
  if (mode == 2) m = FFMode::X2;
  else if (mode == 4) m = FFMode::X4;
  else if (mode == 255) m = FFMode::UNCAPPED;
  g_core.setFastForward(m);
}

JNIEXPORT jboolean JNICALL
Java_com_platinumfast_CoreBridge_stepFrame(JNIEnv*, jobject, jboolean heavy) {
  return g_core.stepFrame(heavy == JNI_TRUE) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_platinumfast_CoreBridge_saveState(JNIEnv* env, jobject, jint slot) {
  auto meta = g_core.snapshotMeta();
  // Pack minimal sections for v1 bring-up: regs + frame meta.
  // Full RAM/VRAM dumps plug in after JIT lands.
  std::vector<SaveSection> secs;
  SaveSection a9{1, std::vector<uint8_t>((uint8_t*)&meta.arm9, (uint8_t*)&meta.arm9 + sizeof(meta.arm9))};
  SaveSection a7{2, std::vector<uint8_t>((uint8_t*)&meta.arm7, (uint8_t*)&meta.arm7 + sizeof(meta.arm7))};
  secs.push_back(std::move(a9));
  secs.push_back(std::move(a7));
  SavestateManager sm(g_baseDir);
  // gamecode "CPUE" enforced by caller after loadRom; fallback keeps file valid.
  return sm.save((uint32_t)slot, "CPUE", meta.frame, secs, nullptr) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_platinumfast_CoreBridge_loadState(JNIEnv*, jobject, jint slot) {
  SavestateManager sm(g_baseDir);
  std::string gc; uint64_t fr = 0;
  std::vector<SaveSection> secs;
  return sm.load((uint32_t)slot, &gc, &fr, &secs) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_platinumfast_CoreBridge_setThermalLevel(JNIEnv*, jobject, jint level) {
  g_core.ff().setThermalLevel((int)level);
}

}  // extern "C"
