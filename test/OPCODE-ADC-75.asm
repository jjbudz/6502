; Opcode $75 must decode as ADC $4B,X (zero page, X) on a real NMOS 6502.
; Emitted as raw bytes so the test checks the CPU's opcode decoding, not the
; assembler's mnemonic table (.DATA takes bare hex digits, no $).
; Pointer ($50) -> $8120 holds $11, while the zero page byte at $50 itself
; is $20, so (zp,X) and zp,X give different results.
$4000   CLC
        LDXI #$05
        LDAI #$20
        STAZ $50
        LDAI #$81
        STAZ $51
        LDAI #$11
        STAA $8120
        LDAI #$30
        .DATA 75 4B
        STAA $8000
        BRK
