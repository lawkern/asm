\\ All comments begin with a single backslach and extend to the end of the
\\ line. Multi-line comments just need a backslash for each line. By convention,
\\ two or more backslashes align to beginning of line, while a single backslash
\\ indents to basic-offset.

\\ Assembler directives begin with '#', in the style of the C preprocessor:
#arch 6502
#section text
#location 0x0100

main: #api
    \ Immediate values have no sigil, and hex literals begin with '0x' (decimal is
    \ the default for non-prefixed integers). Memory references use brackets.

    lda 0x01                    \ Load immediate value into a
    lda [0x01]                  \ Load byte at zero page address into a
    lda [0x01 + x]              \ Load byte at zero page address + x-offset into a
    lda [0x0123]                \ Load byte at absolute address into a
    lda [0x0123 + x]            \ Load byte at absolute address + x-offset into a
    lda [0x0123 + y]            \ Load byte at absolute address + y-offset into a
    lda [[0x01 + x]]            \ Load byte at indexed indirect into a
    lda [[0x01] + y]            \ Load byte at indirect indexed into a

    sta [0x01]                  \ Store a at zero page address
    sta [0x01 + x]              \ Store a at zero page address + x-offset
    sta [0x0123]                \ Store a at absolute address
    sta [0x0123 + x]            \ Store a at absolute address + x-offset
    sta [0x0123 + y]            \ Store a at absolute address + y-offset
    sta [[0x01 + x]]            \ Store a at indexed indirect address
    sta [[0x01] + y]            \ Store a indirect indexed address

    ora 0x01                    \ Bitwise-Or immediate value into a
    ora [0x01]                  \ Bitwise-Or byte at zero page address into a
    ora [0x01 + x]              \ Bitwise-Or byte at zero page address + x-offset into a
    ora [0x0123]                \ Bitwise-Or byte at absolute address into a
    ora [0x0123 + x]            \ Bitwise-Or byte at absolute address + x-offset into a
    ora [0x0123 + y]            \ Bitwise-Or byte at absolute address + y-offset into a
    ora [[0x01 + x]]            \ Bitwise-Or byte at indexed indirect into a
    ora [[0x01] + y]            \ Bitwise-Or byte at indirect indexed into a

    eor 0x01                    \ Bitwise-xor immediate value into a
    eor [0x01]                  \ Bitwise-xor byte at zero page address into a
    eor [0x01 + x]              \ Bitwise-xor byte at zero page address + x-offset into a
    eor [0x0123]                \ Bitwise-xor byte at absolute address into a
    eor [0x0123 + x]            \ Bitwise-xor byte at absolute address + x-offset into a
    eor [0x0123 + y]            \ Bitwise-xor byte at absolute address + y-offset into a
    eor [[0x01 + x]]            \ Bitwise-xor byte at indexed indirect into a
    eor [[0x01] + y]            \ Bitwise-xor byte at indirect indexed into a

    adc 0x01                    \ Add immediate value into a
    adc [0x01]                  \ Add byte at zero page address into a
    adc [0x01 + x]              \ Add byte at zero page address + x-offset into a
    adc [0x0123]                \ Add byte at absolute address into a
    adc [0x0123 + x]            \ Add byte at absolute address + x-offset into a
    adc [0x0123 + y]            \ Add byte at absolute address + y-offset into a
    adc [[0x01 + x]]            \ Add byte at indexed indirect into a
    adc [[0x01] + y]            \ Add byte at indirect indexed into a

    sbc 0x01                    \ Subtract immediate value from a
    sbc [0x01]                  \ Subtract byte at zero page address from a
    sbc [0x01 + x]              \ Subtract byte at zero page address + x-offset from a
    sbc [0x0123]                \ Subtract byte at absolute address from a
    sbc [0x0123 + x]            \ Subtract byte at absolute address + x-offset from a
    sbc [0x0123 + y]            \ Subtract byte at absolute address + y-offset from a
    sbc [[0x01 + x]]            \ Subtract byte at indexed indirect from a
    sbc [[0x01] + y]            \ Subtract byte at indirect indexed from a

    cmp 0x01                    \ Compare immediate value with a
    cmp [0x01]                  \ Compare byte at zero page address with a
    cmp [0x01 + x]              \ Compare byte at zero page address + x-offset with a
    cmp [0x0123]                \ Compare byte at absolute address with a
    cmp [0x0123 + x]            \ Compare byte at absolute address + x-offset with a
    cmp [0x0123 + y]            \ Compare byte at absolute address + y-offset with a
    cmp [[0x01 + x]]            \ Compare byte at indexed indirect with a
    cmp [[0x01] + y]            \ Compare byte at indirect indexed with a

    asl a                       \ Accumulator
    asl [0x01]                  \ Zero page address
    asl [0x01 + x]              \ Zero page address + x-offset
    asl [0x0123]                \ Absolute address
    asl [0x0123 + x]            \ Absolute address + x-offset

    rol a                       \ Accumulator
    rol [0x01]                  \ Zero page address
    rol [0x01 + x]              \ Zero page address + x-offset
    rol [0x0123]                \ Absolute address
    rol [0x0123 + x]            \ Absolute address + x-offset

    lsr a                       \ Accumulator
    lsr [0x01]                  \ Zero page address
    lsr [0x01 + x]              \ Zero page address + x-offset
    lsr [0x0123]                \ Absolute address
    lsr [0x0123 + x]            \ Absolute address + x-offset

    ror a                       \ Accumulator
    ror [0x01]                  \ Zero page address
    ror [0x01 + x]              \ Zero page address + x-offset
    ror [0x0123]                \ Absolute address
    ror [0x0123 + x]            \ Absolute address + x-offset

    stx [0x01]                  \ Zero page address
    stx [0x01 + y]              \ Zero page address + y-offset
    stx [0x0123]                \ Absolute address

    ldx 0x01                    \ Immediate
    ldx [0x01]                  \ Zero page address
    ldx [0x01 + x]              \ Zero page address + x-offset
    ldx [0x0123]                \ Absolute address
    ldx [0x0123 + x]            \ Absolute address + x-offset

    dec [0x01]                  \ Zero page address
    dec [0x01 + x]              \ Zero page address + x-offset
    dec [0x0123]                \ Absolute address
    dec [0x0123 + x]            \ Absolute address + x-offset

    inc [0x01]                  \ Zero page address
    inc [0x01 + x]              \ Zero page address + x-offset
    inc [0x0123]                \ Absolute address
    inc [0x0123 + x]            \ Absolute address + x-offset

    bit [0x01]                  \ Zero page address
    bit [0x0123]                \ Absolute address

    jsr 0x0123                  \ Jump to subroutine address
    jmp 0x0123                  \ Jump to absolute address
    jmp [0x0123]                \ Jump to indirect address

    ldy 0x01                    \ Immediate
    ldy [0x01]                  \ Zero page address
    ldy [0x01 + x]              \ Zero page address + x-offset
    ldy [0x0123]                \ Absolute address
    ldy [0x0123 + x]            \ Absolute address + x-offset

    sty [0x01]                  \ Zero page address
    sty [0x01 + x]              \ Zero page address + x-offset
    sty [0x0123]                \ Absolute address

    cpy 0x01                    \ Immediate
    cpy [0x01]                  \ Zero page address
    cpy [0x0123]                \ Absolute address

    cpx 0x01                    \ Immediate
    cpx [0x01]                  \ Zero page address
    cpx [0x0123]                \ Absolute address

.loop:
    \ Branch to relative address (by immediate)
    bpl 0x01
    bmi 0x01
    bvc 0x01
    bvs 0x01
    bcc 0x01
    bcs 0x01
    bne 0x01
    beq 0x01

    \ Branch to relative address (by label)
    bpl .loop
    bmi .loop
    bvc .loop
    bvs .loop
    bcc .loop
    bcs .loop
    bne .loop
    beq .loop

    brk
    rti
    rts
    php
    plp
    pha
    pla
    dey
    tay
    iny
    inx
    clc
    sec
    cli
    sei
    tya
    clv
    cld
    sed
    txa
    txs
    tax
    tsx
    dex
    nop

#section data
#location 0x2000
#bytes 0x03 0x02 0x01 0x00
