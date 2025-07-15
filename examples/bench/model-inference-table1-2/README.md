
This example evaluates the inference speed on microcontrollers natively, and with WAMR interpreter and WAMR AoT (Ahead-of-Time compilation).

# Generating native model example

To generate native model example, run in the current folder:

```
../../../scripts/create_example_tinyml.bash
cp -r model-template/* model
```

# Usage

In each subfolder, you can:

- run `BOARD=arduino-nano-33-ble make clean all flash term` to flash the inference and evaluation code on your board. It will display the time required to do inference with DS CNN model. 
    - You can replace `arduino-nano-33-ble` with your board, the full list of supported boards is available in [RIOT-OS documentation](https://doc.riot-os.org/group__boards.html).
- run `BOARD=arduino-nano-33-ble make clean bench-size` to display the size of the different components in the produced image.