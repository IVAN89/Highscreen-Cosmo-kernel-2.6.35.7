cmd_drivers/mtd/tests/mtd_stresstest.ko := arm-eabi-ld -EL -r  -T /media/source/cosmo/scripts/module-common.lds --build-id -o drivers/mtd/tests/mtd_stresstest.ko drivers/mtd/tests/mtd_stresstest.o drivers/mtd/tests/mtd_stresstest.mod.o