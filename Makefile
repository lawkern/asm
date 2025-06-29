CFLAGS = -g -Wall -Wextra -Wno-unused-function

compile:
	mkdir -p build
	$(CC) -o build/asm_6502   $(CFLAGS) -DARCH_6502   src/main.c $(LDFLAGS)
	$(CC) -o build/asm_armv4t $(CFLAGS) -DARCH_ARMV4T src/main.c $(LDFLAGS)
	$(CC) -o build/asm_armv8  $(CFLAGS) -DARCH_ARMV8  src/main.c $(LDFLAGS)

run:
	cd build; ./asm_6502   ../data/example_6502_00.asm
	cd build; ./asm_armv4t ../data/example_arm32_00.asm
	cd build; ./asm_armv8  ../data/example_arm64_00.asm
