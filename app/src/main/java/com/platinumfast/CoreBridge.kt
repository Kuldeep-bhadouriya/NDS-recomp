package com.platinumfast

// JNI surface: mirrors app/src/main/cpp/jni_bridge.cpp exactly.
// ROM/BIOS never bundled — user picks their own CPUE dump via SAF.
object CoreBridge {
  init { System.loadLibrary("platinum_jni") }
  external fun loadRom(path: String): Boolean
  external fun unload()
  external fun setFastForward(mode: Int) // 0,2,4,255
  external fun stepFrame(is3DHeavy: Boolean): Boolean
  external fun saveState(slot: Int): Boolean
  external fun loadState(slot: Int): Boolean
  external fun setThermalLevel(level: Int) // 0 normal, 1 moderate, 2 severe
}
