#!/bin/bash
# Builds zygisk/arm64-v8a.so. Dobby master is currently broken on Android,
# so try dated snapshots of its history until one compiles.
set -u
NDK="${ANDROID_NDK_LATEST_HOME:-${ANDROID_NDK_HOME:-}}"
cd "$(dirname "$0")"
curl -fsSL https://raw.githubusercontent.com/topjohnwu/zygisk-module-sample/master/module/jni/zygisk.hpp -o zygisk.hpp || { echo "zygisk.hpp download failed"; exit 1; }
rm -rf Dobby && git clone -q https://github.com/jmpews/Dobby.git Dobby || exit 1
for D in 2023-06-01 2022-12-01 2022-06-01; do
  echo "=== Dobby snapshot before $D ==="
  REV=$(cd Dobby && git rev-list -1 --before="$D" HEAD)
  [ -n "$REV" ] || continue
  (cd Dobby && git checkout -qf "$REV") || continue
  find Dobby -name 'closure_bridge_arm64.asm' -exec sed -i 's/\(common_closure_bridge_handler\)@PAGEOFF/:lo12:\1/; s/\(common_closure_bridge_handler\)@PAGE/\1/' {} +
  rm -rf build
  if cmake -B build -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
       -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static \
       -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
     && cmake --build build -j"$(nproc)"; then
    mkdir -p ../zygisk
    cp build/arm64-v8a.so ../zygisk/arm64-v8a.so
    "$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip" ../zygisk/arm64-v8a.so || true
    echo "Built with Dobby $REV"
    exit 0
  fi
done
echo "All Dobby snapshots failed"
exit 1
