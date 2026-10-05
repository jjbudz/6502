; Test BCD Subtraction - Basic case
; Tests: SBC in decimal mode
; Expected: $25 - $16 = $09 in BCD (with carry set for no borrow)
; Result stored at $8000 should be $09
$4000   SED
        SEC           ; Set carry (no borrow)
        LDAI #$25
        SBCI #$16
        CMPI #$09
        BNE fail
        STAA $8000
        CLD
        BRK
fail    LDAI #$FF
        STAA $8000
        CLD
        BRK
