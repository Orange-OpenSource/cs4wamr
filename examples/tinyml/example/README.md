# Example

This project is an example of the usage of the framework on an tinyml usecase.

# Building project

You must first setup RIOT ML and Apache TVM using:
```bash
source ./scripts/setup_riotml.bash
```


Then, you should generate the WebAssembly tinyml container using:
```
make configure_model
```

Finally, you can build and flash the project using:

```bash
BOARD=arduino-nano-33-ble make build flash
```

You can replace `arduino-nano-33-ble` with your board, the full list of supported boards is available in [RIOT-OS documentation](https://doc.riot-os.org/group__boards.html).
