compile:
	gprbuild -Pasm.gpr -j0 -cargs $(CFLAGS) -largs $(LDFLAGS)

run:
	./build/asm
