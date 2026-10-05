; Test BCD Addition - Basic case
; Tests: ADC in decimal mode
; Expected: $09 + $01 = $10 in BCD (not $0A)
; Result stored at $8000 should be $10
$4000   SED           ; Set decimal mode
        CLC
        LDAI #$09
        ADCI #$01
        STAA $8000
        CLD           ; Clear decimal mode
        BRK
