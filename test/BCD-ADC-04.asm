; Test BCD Addition - With carry in
; Tests: ADC in decimal mode with carry flag set
; Expected: $15 + $25 + carry(1) = $41 in BCD
; Result stored at $8000 should be $41
$4000   SED
        SEC           ; Set carry before addition
        LDAI #$15
        ADCI #$25
        CMPI #$41     ; 15 + 25 + 1 = 41
        BNE fail
        STAA $8000
        CLD
        BRK
fail    LDAI #$FF
        STAA $8000
        CLD
        BRK
