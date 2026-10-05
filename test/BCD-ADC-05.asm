; Test BCD Addition - Edge case with zeros
; Tests: ADC in decimal mode with zero
; Expected: $00 + $00 = $00 in BCD
; Result stored at $8000 should be $00
$4000   SED
        CLC
        LDAI #$00
        ADCI #$00
        CMPI #$00
        BNE fail
        BCS fail2     ; No carry should be generated
        STAA $8000
        CLD
        BRK
fail    LDAI #$FE
        STAA $8000
        CLD
        BRK
fail2   LDAI #$FD
        STAA $8000
        CLD
        BRK
