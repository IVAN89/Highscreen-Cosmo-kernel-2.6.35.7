cmd_drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o := arm-eabi-gcc -Wp,-MD,drivers/cci/sensors/yamaha/geomagnetic/.yas_mag_driver-yas529.o.d  -nostdinc -isystem /home/ivan/android-ndk-r5/toolchains/arm-eabi-4.4.0/prebuilt/linux-x86/bin/../lib/gcc/arm-eabi/4.4.0/include -I/media/source/cosmo/arch/arm/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -DCCI_GFIVE=0 -DCCI_ONDA=0 -DCCI_VOBIS=0 -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -mlittle-endian -Iarch/arm/mach-msm/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -Werror -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -DCCI_PHYS_OFFSET=0x02E00000 -DCCI_IOMAP_SHIFT=0x16000000 -Os -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(yas_mag_driver_yas529)"  -D"KBUILD_MODNAME=KBUILD_STR(yas_mag_driver_yas529)" -D"DEBUG_HASH=22" -D"DEBUG_HASH2=28" -c -o drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.c

deps_drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o := \
  drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.c \
  drivers/cci/sensors/yamaha/geomagnetic/yas.h \
  drivers/cci/sensors/yamaha/geomagnetic/yas_cfg.h \

drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o: $(deps_drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o)

$(deps_drivers/cci/sensors/yamaha/geomagnetic/yas_mag_driver-yas529.o):
