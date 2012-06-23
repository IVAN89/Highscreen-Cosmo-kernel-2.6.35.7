cmd_drivers/bluetooth/built-in.o :=  arm-eabi-ld -EL    -r -o drivers/bluetooth/built-in.o drivers/bluetooth/hci_uart.o drivers/bluetooth/msm_bt_sleep.o drivers/bluetooth/bluetooth-power.o 
