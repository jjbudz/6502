; ADC ($FF),Y - the pointer's high byte must come from $00, not $0100.
; ($FF)=$34, ($00)=$80 -> $8034+Y; a decoy high byte $90 sits at $0100.
$4000   LDAI #$34
        STAZ $FF
        LDAI #$80
        STAZ $00
        LDAI #$90
        STAA $0100
        LDAI #$11
        STAA $8036
        LDAI #$22
        STAA $9036
        LDYI #$02
        CLC
        LDAI #$30
        ADCIY $FF
        STAA $8000
        BRK
