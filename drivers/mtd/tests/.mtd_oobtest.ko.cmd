cmd_drivers/mtd/tests/mtd_oobtest.ko := arm-eabi-ld -EL -r  -T /media/source/cosmo/scripts/module-common.lds --build-id -o drivers/mtd/tests/mtd_oobtest.ko drivers/mtd/tests/mtd_oobtest.o drivers/mtd/tests/mtd_oobtest.mod.o