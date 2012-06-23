cmd_lib/decompress.o := arm-eabi-gcc -Wp,-MD,lib/.decompress.o.d  -nostdinc -isystem /home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/../lib/gcc/arm-eabi/4.4.0/include -I/media/source/cosmo/arch/arm/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -DCCI_GFIVE=0 -DCCI_ONDA=0 -DCCI_VOBIS=0 -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mlittle-endian -Iarch/arm/mach-msm/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -Werror -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -Os -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(decompress)"  -D"KBUILD_MODNAME=KBUILD_STR(decompress)" -D"DEBUG_HASH=17" -D"DEBUG_HASH2=2" -c -o lib/decompress.o lib/decompress.c

deps_lib/decompress.o := \
  lib/decompress.c \
    $(wildcard include/config/decompress/gzip.h) \
    $(wildcard include/config/decompress/bzip2.h) \
    $(wildcard include/config/decompress/lzma.h) \
    $(wildcard include/config/decompress/lzo.h) \
  include/linux/decompress/generic.h \
  include/linux/decompress/bunzip2.h \
  include/linux/decompress/unlzma.h \
  include/linux/decompress/inflate.h \
  include/linux/decompress/unlzo.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /media/source/cosmo/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /media/source/cosmo/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /media/source/cosmo/arch/arm/include/asm/posix_types.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/../lib/gcc/arm-eabi/4.4.0/include/stdarg.h \
  /media/source/cosmo/arch/arm/include/asm/string.h \

lib/decompress.o: $(deps_lib/decompress.o)

$(deps_lib/decompress.o):
