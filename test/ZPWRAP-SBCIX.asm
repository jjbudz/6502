; SBC ($FE,X) with X=1 - the pointer at $FF must take its high byte from $00,
; not $0100. ($FF)=$34, ($00)=$80 -> $8034; a decoy high byte $90 sits at $0100.
$4000   LDAI #$34
        STAZ $FF
        LDAI #$80
        STAZ $00
        LDAI #$90
        STAA $0100
        LDAI #$11
        STAA $8034
        LDAI #$22
        STAA $9034
        LDXI #$01
        SEC
        LDAI #$30
        SBCIX $FE
        STAA $8000
        BRK
