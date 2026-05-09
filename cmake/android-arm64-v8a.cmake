# Thin wrapper — delegates to NDK's own toolchain, overrides ABI/API/STL
# ANDROID_NDK must be passed via -DANDROID_NDK=...
set(ANDROID_ABI arm64-v8a)
set(ANDROID_PLATFORM android-21)
set(ANDROID_STL c++_static)

if(NOT DEFINED ANDROID_NDK)
  if(DEFINED ENV{ANDROID_NDK_HOME})
    set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
  elseif(DEFINED ENV{ANDROID_NDK})
    set(ANDROID_NDK $ENV{ANDROID_NDK})
  else()
    message(FATAL_ERROR "ANDROID_NDK not set")
  endif()
endif()

include("${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
