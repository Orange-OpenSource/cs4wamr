APPLICATION = tiny-ml-usecase
BOARD ?= arduino-nano-33-ble

BASE_DIR ?= ../../..
RIOTBASE ?= $(BASE_DIR)/dependencies/RIOT
SWITCH_INJECTION_TOOL ?= $(BASE_DIR)/src/elf-reader/reader.py 
CUST_MODULE_DIR ?=$(BASE_DIR)/src/modules

EXTERNAL_MODULE_DIRS += $(CUST_MODULE_DIR)

USEPKG += wamr
USEMODULE += ztimer ztimer_sec benchmark
USEMODULE += shell ps shell_cmds_default 
USEMODULE += sched_round_robin wamr_env_thread
 
export ENABLE_AOT ?= 1

ifeq ($(ENABLE_AOT),1)
	export WAMR_CONFIG := $(abspath wamr_config_aot.cmake)
	CFLAGS += -DENABLE_AOT=1
else
	export WAMR_CONFIG := $(abspath wamr_config.cmake)
endif

CFLAGS += -DTHREAD_STACKSIZE_MAIN='(20 * 1024)' -Wno-unused-parameter -Wno-unused-variable -DSCHED_RR_TIMEOUT=500

DEVELHELP ?= 1
QUIET ?= 1

WARN_UNUSED_SWAP ?= 1

#######################

all: clean_wasm_module check_warn compile_wasm_module
clean: clean_wasm_module

build: WARN_UNUSED_SWAP=0
build: all update-elf update-bin


configure_model:
	bash ./scripts/generate_model_dir.bash

setup_riotml:
	bash ./scripts/setup_riotml.bash

compile_wasm_module:
	make -C wasm

clean_wasm_module:
	make -C wasm clean

update-elf:
	python3 $(SWITCH_INJECTION_TOOL) --elf $(ELFFILE) --wamr $(RIOTBASE)/build/pkg/wamr/

update-bin: 
	$(Q)$(OBJCOPY) $(OFLAGS) -Obinary $(ELFFILE) $(BINFILE)
#	 echo ESPTOOL: $(ESPTOOL)
#	 $(Q)$(ESPTOOL) --chip $(FLASH_CHIP) elf2image --flash_mode $(FLASH_MODE) \
#		--flash_size $(FLASH_SIZE)MB --flash_freq $(FLASH_FREQ) $(FLASH_OPTS) \
#		-o $(ELFFILE).bin $(ELFFILE)

check_warn:
	@[ $(WARN_UNUSED_SWAP) -eq 1 ] && \
		echo "\nSWAP is not used !\nPleaser, use 'make build ...' to enable swapping or set WARN_UNUSED_SWAP env variable to 0.\n'make build' run the firmware analyzer script after compiling the firmware and update the firmware to provide the necessary informations to the context switch library." && exit 2 || true

run: build flash-only term

.PHONY: all compile_wasm_module

#############################

include $(RIOTBASE)/Makefile.include

default: build
.DEFAULT_GOAL := default