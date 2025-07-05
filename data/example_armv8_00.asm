\ This comment goes until the end of the line.

#file example_armv8_00.bin

#arch armv8
#section text

main:
    mov r1, 0xAB      \ As does this one.
    mov r2, 0xBC      \ And this one too.
    add r0, r1, r2

.loop: b .loop
