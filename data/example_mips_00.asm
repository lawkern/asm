#file example_mips_00.bin

#architecture mips32

#bytes  0x80 0x37 0x12 0x40    \ Configuration flags (4 bytes)
#4bytes 0x00000000             \ CPU clock rate (4 bytes)
#4bytes main                   \ Boot address (4 bytes)
#4bytes 0x00000000             \ libultra version (unused) (4 bytes)
#string "REMOVEME"             \ Check code (8 bytes)
#8bytes 0                      \ Reserved (8 bytes)
#string "N64 TEST EXE        " \ Game title (20 bytes)
#bytes  0 0 0 0 0 0 0          \ Reserved (7 bytes)
#string "N"                    \ Category code (Game Pak) (1 byte)
#string "HN"                   \ Unique code (2 bytes)
#string "E"                    \ Destination code (North America) (1 byte)
#bytes  0                      \ ROM Version (1 byte)

main:
    j main
    nop
