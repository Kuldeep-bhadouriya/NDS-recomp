cmake_minimum_required(VERSION 3.22)
project(platinum_fast)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 16KB page-size + W^X ready: no TEXTREL, no executable stack.
add_library(platinum_core STATIC
  core/src/fast_forward.cpp
  core/src/savestate.cpp
  core/src/memory_bus.cpp
  core/src/jit.cpp
  core/src/gpu.cpp
  core/src/snd.cpp
  core/src/emulator_core.cpp
)
target_include_directories(platinum_core PUBLIC core/include)

if(ANDROID)
  add_library(platinum_jni SHARED app/src/main/cpp/jni_bridge.cpp)
  # Link PT_LOAD segments at 16KB alignment so the .so loads on
  # 16KB-page-size devices (Android 15+/new SoCs) without remapping.
  target_link_options(platinum_jni PRIVATE
    "-Wl,-z,max-page-size=16384"
    "-Wl,-z,common-page-size=16384")
  target_link_libraries(platinum_jni platinum_core log)
endif()
