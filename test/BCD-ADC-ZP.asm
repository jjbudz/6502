; Test BCD ADC through a non-immediate addressing mode
; Tests: SED; $55 + $45 (zero page operand) = 100 -> A=$00 with carry
;        Exercises the shared decimal path from ADC zero page
; Expected: A=$00, C=1
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$45
        STAZ $10
        SED
        CLC
        LDAI #$55
        ADCZ $10
        BCS ok1       ; C must be set
        BRK
ok1     CMPI #$00
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
