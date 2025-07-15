
# Usage

In each subfolder, you can:
- Edit the makefile to set the number of container (DNB_CONTAINERS) between 1 and 2.
- run `BOARD=arduino-nano-33-ble make clean build flash term` to flash your device with the example.
- run `BOARD=arduino-nano-33-ble make clean build bench-size` to display the size of the different components in the produced image.