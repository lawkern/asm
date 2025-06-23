CFLAGS = -g -Wall -Wextra -Wno-unused-function

compile:
	mkdir -p build
	$(CC) -o build/asm_6502   $(CFLAGS) -DARCH_6502   src/main.c $(LDFLAGS)
	$(CC) -o build/asm_arm4vt $(CFLAGS) -DARCH_ARM4VT src/main.c $(LDFLAGS)

run:
	./build/asm_6502 data/example_6502_00.asm
#	./build/asm_arm4vt data/example_arm32_00.asm
