compile:
	mkdir -p build
	$(CC) -o build/asm $(CFLAGS) src/main.c $(LDFLAGS)

run:
	./build/asm data/example_arm32_00.asm
