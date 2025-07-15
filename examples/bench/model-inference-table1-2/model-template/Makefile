
EXTERNAL_PKG_DIRS += models

USEPKG += default 
USEMODULE += ztimer random stdin

USEMODULE += mlmci
EXTERNAL_MODULE_DIRS += $(CURDIR)

CFLAGS += -Wno-strict-prototypes 
# CFLAGS += -Wno-missing-include-dirs
CFLAGS += -Wno-discarded-qualifiers -O3 -emit-llvm -flto

LINKER_FLAGS += --lto-O3 -O3

# LTO = 1


INJECT_FOR_SWAP ?= 0
WARN_UNUSED_SWAP ?= 0
USE_WAMR_ENV_THREAD ?= 0
USE_WAMR_ENV ?= 0
USE_SHELL = 1

DEVELHELP ?= 0

APPLICATION = wasm-example
BOARD ?= arduino-nano-33-ble

BASE_DIR ?= ../../../..
# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(BASE_DIR)/dependencies/RIOT

USEMODULE += ztimer ztimer_sec benchmark


SCRIPTS_DIRS += $(BASE_DIR)/scripts

bench-size: clean all
#	 $(TERMPROG) $(TERMFLAGS) | xargs -L1 echo "a"
	python3 $(SCRIPTS_DIRS)/evaluate_size.py $(ELFFILE) -c module $(EXTERNAL_MODULE_DIRS) -c example $(APPDIR) -c wamr $(RIOTBASE)/build/pkg/wamr

include $(RIOTBASE)/Makefile.include
