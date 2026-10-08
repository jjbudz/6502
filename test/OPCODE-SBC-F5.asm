; Opcode $F5 must decode as SBC $4B,X (zero page, X) on a real NMOS 6502.
; Emitted as raw bytes so the test checks the CPU's opcode decoding, not the
; assembler's mnemonic table.
; Pointer ($50) -> $8120 holds $11, while the zero page byte at $50 itself
; is $20, so (zp,X) and zp,X give different results.
$4000   SEC
        LDXI #$05
        LDAI #$20
        STAZ $50
        LDAI #$81
        STAZ $51
        LDAI #$11
        STAA $8120
        LDAI #$30
        .DATA $F5 $4B
        STAA $8000
        BRK
