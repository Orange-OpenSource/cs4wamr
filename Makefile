
all:
	make -C examples/demo clean build
	make -C examples/tinyml setup_riotml
	make -C examples/tinyml clean build

test:


