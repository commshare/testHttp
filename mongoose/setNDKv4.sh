#!/bin/sh
echo "set ndk env for vlc android build "
NDK_ROOT=/home/zhangbin/android
NDK_VERSION=android-ndk-r9d
NDK_CUR=$NDK_ROOT/$NDK_VERSION


export ANDROID_NDK=$NDK_CUR

echo "ANDROID_NDK IS $ANDROID_NDK"
echo "============================="
SDK_HOME=/home/zhangbin/android
SDK_VER=sdk
SDK_NAME=android-sdk-linux
SDK_CUR=$SDK_HOME/$SDK_VER/$SDK_NAME
export ANDROID_SDK=$SDK_CUR

echo "ANDROID_SDK IS $ANDROID_SDK"

echo "============================="

#export PATH=$PATH:$ANDROID_SDK/platform-tools:$ANDROID_SDK/tools:$PATH
#echo "PATH IS $PATH"

echo "=============================="
#You need to export the ABI for your device. armeabi-v7a is for devices wit#h ARMv7 or above (regardless of NEON), any other ARM device uses armeab

#export ANDROID_ABI=armeabi-v7a
#=====above from vlc-android ==================

###add ndk-build to path
NDK_BUILD_PATH=$NDK_CUR

export PATH=$PATH:$NDK_BUILD_PATH:$ANDROID_SDK/platform-tools:$ANDROID_SDK/tools:$PATH
echo "PATH IS $PATH"

#########add standalone toolchain ####
#pwd/home/zhangbin/android/android-toolchain/r9d-4.8-standalone/android-14
TOOL_HOME=$NDK_ROOT/android-toolchain
TOOL_NAME=r9d-4.8-standalone
TOOL_API=android-17
TOOL_CHAIN=$TOOL_HOME/$TOOL_NAME/$TOOL_API

export ANDROID_TOOLCHAIN=$TOOL_CHAIN

echo "ANDROID_TOOLCHAIN is $ANDROID_TOOLCHAIN"

export PATH=$ANDROID_TOOLCHAIN/bin:$PATH
echo "PATH IS $PATH"
