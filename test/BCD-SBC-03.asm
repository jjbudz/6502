; Test BCD Subtraction - Zero result
; Tests: SBC in decimal mode giving zero
; Expected: $50 - $50 = $00 in BCD
; Result stored at $8000 should be $00
$4000   SED
        SEC
        LDAI #$50
        SBCI #$50
        CMPI #$00
        BNE fail
        STAA $8000
        CLD
        BRK
fail    LDAI #$FF
        STAA $8000
        CLD
        BRK
