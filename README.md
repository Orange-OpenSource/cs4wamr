
# CS4WAMR (Context Switching for WAMR)

## Overview

The repository proposes a toolkit which implements a configuration switching mechanism to allows running multiple instances of [WebAssembly Micro Runtime (WAMR)](https://github.com/bytecodealliance/wasm-micro-runtime/) at the same time on single-core microcontrollers. 


### Context-switching mechanism

The mechanism consists of running multiples instances of WAMR by saving the current instance, and creating or loading a new one. 


### Components

To use the mechanism, the tool is composed in two components:

- a Firmware Analyzer Tool: to analyze the produce firmware and recuperate information about the memory of WAMR and inject these information in our library
- a library to perform the context switch. The library is composed in three sub-libraries:
    - static_context_switcher: to perform the swapping,
    - wamr_env: to use multiple instances of WAMR by automatically using multi-static,
    - wamr_env_thread: to use wamr_env for having dedicated thread for the environments.

## Usage

Clone the project with submodules using 

```
git clone --recursive https://github.com/Orange-OpenSource/cs4wamr.git
```

### Examples

We propose two examples of usage of the toolkit: 

- [tiny-ml](./examples/tinyml/): an example of using the toolkit to run machine learning inference on microcontrollers among other applications.
- [demo](./examples/demo): an example of using the toolkit to run concurrently multiple containers.

We also propose some performance evaluation on both using the framework and using WAMR directly. Such evaluations are:
- [model-inference](./examples/bench/model-inference-table1-2/): evaluation of inference time natively with microcontrollers, with WAMR interpreter and with WAMR AoT.
- [sizes](./examples/bench/sizes-table3/): measurement of size usage by WAMR and CS4WAMR.
- [switch](./examples/bench/sizes-table3/): evaluation of CS4WAMR performances on switch and snapshot.



## Organization

### Folders

- [`/dependencies`](./dependencies/): dependencies used by the repository. See [this](https://git-scm.com/book/en/v2/Git-Tools-Submodules#_cloning_submodules) to learn how to use the git submodules.
- [`/examples`](./examples/): Example of application to use the WAMR Context Switch Toolkit.
- [`/scripts`](./scripts/): Scripts to setup and evaluate the project.
- [`/src`](./src/): Source code of the framework.


## Supported environment

The toolkit and examples are made for the [RIOT-OS](https://github.com/RIOT-OS/RIOT). But the toolkit can easily be adapted to other platforms.

The library has been tested on the Arduino Nano 33 BLE, the DWM1001 and the nRF9160DK.

## License

Copyright (c) Orange

This code is released under the MIT License. See the [LICENSE](./LICENCE) file for more information.

## Contributors

- Bastien BUIL: Conceptualization and Software
- Samuel LEGOUIX: Conceptualization
- Chrystel GABER: Conceptualization