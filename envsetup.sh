#!/bin/sh

# export PATH=$PATH:/home/deus/hack/cyanogen/gb/prebuilt/darwin-x86/toolchain/arm-eabi-4.4.3/bin/
export CCI_MEM_MAPPING=true
# put correct path to your toolchain
export PATH=$PATH:/home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/
export ARCH=arm
export KERNEL_DIR=`pwd`
export CROSS_COMPILE=arm-eabi-
