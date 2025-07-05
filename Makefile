CFLAGS = -g -Wall -Wextra -Wno-unused-function -Wno-unused-variable

compile:
	mkdir -p build
	$(CC) -o build/asm_6502  -DARCH_6502  $(CFLAGS) src/main.c $(LDFLAGS)
	$(CC) -o build/asm_armv4 -DARCH_ARMV4 $(CFLAGS) src/main.c $(LDFLAGS)
	$(CC) -o build/asm_armv8 -DARCH_ARMV8 $(CFLAGS) src/main.c $(LDFLAGS)

run:
	build/asm_6502  data/example_6502_00.asm
	build/asm_armv4 data/example_armv4_00.asm
	build/asm_armv8 data/example_armv8_00.asm
