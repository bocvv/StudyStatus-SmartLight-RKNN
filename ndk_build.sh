# NDK_PROJECT_PATH决定主目录，因为我们的依赖库都放在上层，编译中间结果和生成的库文件也将位于主目录下
#ndk-build NDK_PROJECT_PATH=../.. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk TARGET_ARCH_ABI=armeabi-v7a APP_ABI=armeabi-v7a
ndk-build NDK_PROJECT_PATH=build_for_android APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk TARGET_ARCH_ABI=arm64-v8a APP_ABI=arm64-v8a
