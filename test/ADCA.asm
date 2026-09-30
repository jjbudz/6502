; Test ADC (Add with Carry) - Absolute addressing mode
; Tests: ADC $addr
; Expected: A = $30 + $50 = $80 (with carry clear)
; Result stored at $8000 should be $80
$4000   CLC
        LDAI #$50
        STAA $8100
        LDAI #$30
        ADCA $8100
        STAA $8000
        BRK
