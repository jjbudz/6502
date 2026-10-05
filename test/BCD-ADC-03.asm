; Test BCD Addition - Multi-digit carry
; Tests: ADC in decimal mode with digit carry
; Expected: $58 + $46 = $04 with carry (104 in decimal)
; Result stored at $8000 should be $04, carry set
$4000   SED
        CLC
        LDAI #$58
        ADCI #$46
        BCC fail      ; Carry should be set (58+46=104)
        CMPI #$04
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
