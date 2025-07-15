WEBASSEMBLY_GLOBALBASE=16 
WEBASSEMBLY_STACKSIZE=1024

COMPATIBILITY_CODE=imports/libc/compatibility-code
SYSROOT=imports/libc/wasm-sysroot


CFILES = model_iovars.c main.c model_params.c model_registry.c mlmci/mlmci_global.c mlmci/mlmci_impl.c \
		 bin/gen-twm/codegen/host/src/default_lib0.c bin/gen-twm/codegen/host/src/default_lib1.c \
		 $(COMPATIBILITY_CODE)/implem.c

INCLUDES = -Ibin/gen-twm/runtime/include \
		-Ibin/gen-twm/codegen/host/include \
		-Imlmci/include \
		-I$(SYSROOT)/include \
		-I$(COMPATIBILITY_CODE)/include \

CCFLAGS = 	--target=wasm32-none-none-wasm  \
			-fno-builtin \
			-Os -flto -fvisibility=hidden -Wall \
			-nostdlib \
			$(INCLUDES)


LDFLAGS =	-Wl,--no-entry -Wl,--allow-undefined-file=$(SYSROOT)/share/defined-symbols.txt\
	    	-Wl,--export=get_input_address -Wl,--export=get_output_address -Wl,--export=run -Wl,--export=init   \
	    	-Wl,--export=__heap_base -Wl,--export=__data_end \
			-Wl,--global-base=$(WEBASSEMBLY_GLOBALBASE) \
			-Wl,--strip-all -Wl,--no-entry, -Wl,--lto-O3  \
			-z stack-size=$(WEBASSEMBLY_STACKSIZE) \


all: extract build_wasm opt

extract:
	mkdir -p bin/gen-twm
	cd bin/gen-twm && tar -xvf ../../models/default/default.tar

build_wasm: extract
	clang $(CFILES) \
		-o out.wasm \
	    $(CCFLAGS) \
		$(LDFLAGS) \
		--sysroot $(SYSROOT) \


opt:
	/opt/binaryen-version_119/bin/wasm-opt out.wasm -Oz -o output.wasm  

clean:
	rm -rf output.wasm out.wasm bin
