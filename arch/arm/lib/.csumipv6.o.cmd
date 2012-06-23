cmd_arch/arm/lib/csumipv6.o := arm-eabi-gcc -Wp,-MD,arch/arm/lib/.csumipv6.o.d  -nostdinc -isystem /home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/../lib/gcc/arm-eabi/4.4.0/include -I/media/source/cosmo/arch/arm/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -DCCI_GFIVE=0 -DCCI_ONDA=0 -DCCI_VOBIS=0 -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mlittle-endian -Iarch/arm/mach-msm/include -D__ASSEMBLY__ -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables  -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -include asm/unified.h -msoft-float -gdwarf-2       -c -o arch/arm/lib/csumipv6.o arch/arm/lib/csumipv6.S

deps_arch/arm/lib/csumipv6.o := \
  arch/arm/lib/csumipv6.S \
  /media/source/cosmo/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /media/source/cosmo/arch/arm/include/asm/linkage.h \
  /media/source/cosmo/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
  /media/source/cosmo/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /media/source/cosmo/arch/arm/include/asm/hwcap.h \

arch/arm/lib/csumipv6.o: $(deps_arch/arm/lib/csumipv6.o)

$(deps_arch/arm/lib/csumipv6.o):
