cmd_crypto/ansi_cprng.ko := arm-eabi-ld -EL -r  -T /media/source/cosmo/scripts/module-common.lds --build-id -o crypto/ansi_cprng.ko crypto/ansi_cprng.o crypto/ansi_cprng.mod.o
