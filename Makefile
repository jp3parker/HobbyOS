

.PHONY: all build headers iso run clean

all:
	./clean.sh
	./headers.sh
	./iso.sh
	./qemu.sh

build:
	./build.sh

headers:
	./headers.sh

iso:
	./iso.sh

run:
	./qemu.sh

clean:
	./clean.sh
