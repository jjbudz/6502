; Test BCD ADC zero edge case
; Tests: SED; $00 + $00 = $00 with no carry
; Note: carry is checked before CMPI, since CMP itself sets C when A >= M
; Expected: A=$00, C=0, Z=1
; Result stored at $8000 should be $01 (success)
$4000   SED
        CLC
        LDAI #$00
        ADCI #$00
        BCC ok1       ; C must be clear
        BRK
ok1     BEQ ok2       ; Z must be set (binary sum is zero)
        BRK
ok2     CMPI #$00
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
