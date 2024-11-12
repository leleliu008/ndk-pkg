#!/bin/sh

set -ex

export ANDROID_NDK_CC="$HOME/.ndk-pkg/uppm/installed/android-ndk-r26d/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang"
export ANDROID_NDK_COMPILER_ARGS="--target=aarch64-linux-android21 --sysroot=$HOME/.ndk-pkg/uppm/installed/android-ndk-r26d/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"
export NDKPKG_VERBOSE=1

cc -flto -Os -o wrapper-target-cc wrapper-target-cc.c

./wrapper-target-cc -flto -Os -o test wrapper-target-cc.c
