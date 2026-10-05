; Test BCD Addition - Carry generation
; Tests: ADC in decimal mode with carry
; Expected: $99 + $01 = $00 with carry set in BCD
; Result stored at $8000 should be $00, carry flag set
$4000   SED           ; Set decimal mode
        CLC
        LDAI #$99
        ADCI #$01
        BCC fail      ; Carry should be set
        STAA $8000    ; Should be $00
        CLD
        BRK
fail    LDAI #$FF
        STAA $8000
        CLD
        BRK
