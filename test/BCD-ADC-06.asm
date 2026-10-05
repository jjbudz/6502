; Test BCD Addition - Maximum digits
; Tests: ADC in decimal mode with $99 + $99
; Expected: $99 + $99 = $98 with carry in BCD (198 decimal)
; Result stored at $8000 should be $98, carry set
$4000   SED
        CLC
        LDAI #$99
        ADCI #$99
        BCC fail      ; Carry should be set
        CMPI #$98     ; Lower two digits of 198
        BNE fail2
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
