cmd_arch/arm/vfp/vfphw.o := arm-eabi-gcc -Wp,-MD,arch/arm/vfp/.vfphw.o.d  -nostdinc -isystem /home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/../lib/gcc/arm-eabi/4.4.0/include -I/media/source/cosmo/arch/arm/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -DCCI_GFIVE=0 -DCCI_ONDA=0 -DCCI_VOBIS=0 -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mlittle-endian -Iarch/arm/mach-msm/include -D__ASSEMBLY__ -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -include asm/unified.h -Wa,-mfpu=softvfp+vfp -gdwarf-2       -c -o arch/arm/vfp/vfphw.o arch/arm/vfp/vfphw.S

deps_arch/arm/vfp/vfphw.o := \
  arch/arm/vfp/vfphw.S \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/thumb2/kernel.h) \
    $(wildcard include/config/vfpv3.h) \
  /media/source/cosmo/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
  /media/source/cosmo/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /media/source/cosmo/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/iwmmxt.h) \
  /media/source/cosmo/arch/arm/include/asm/vfpmacros.h \
  /media/source/cosmo/arch/arm/include/asm/vfp.h \
  arch/arm/vfp/../kernel/entry-header.S \
    $(wildcard include/config/frame/pointer.h) \
    $(wildcard include/config/alignment/trap.h) \
    $(wildcard include/config/cpu/32v6k.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/linkage.h \
  /media/source/cosmo/arch/arm/include/asm/linkage.h \
  /media/source/cosmo/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/trace/irqflags.h) \
  /media/source/cosmo/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
  /media/source/cosmo/arch/arm/include/asm/hwcap.h \
  /media/source/cosmo/arch/arm/include/asm/asm-offsets.h \
  include/generated/asm-offsets.h \
  /media/source/cosmo/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \

arch/arm/vfp/vfphw.o: $(deps_arch/arm/vfp/vfphw.o)

$(deps_arch/arm/vfp/vfphw.o):
