#!/bin/bash
set -e
NDK="${ANDROID_NDK_LATEST_HOME:-$ANDROID_NDK_HOME}"
cd "$(dirname "$0")"
DBG=OFF; [ "$1" = "debug" ] && DBG=ON
curl -fsSL https://raw.githubusercontent.com/topjohnwu/zygisk-module-sample/master/module/jni/zygisk.hpp -o zygisk.hpp
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release -DSG_DEBUG=$DBG
cmake --build build -j"$(nproc)"
mkdir -p ../zygisk
cp build/arm64-v8a.so ../zygisk/arm64-v8a.so
"$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" ../zygisk/arm64-v8a.so || true
