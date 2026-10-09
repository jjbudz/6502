; The stack lives in page 1: with SP=$FF, PHA writes $01FF and JSR writes
; its return address to $01FE/$01FD (high byte first). Reads them back as
; ordinary memory and stores (value + PCH + PCL) at $8000.
        .ORG $4000
        LDA #$42
        PHA             ; $01FF = $42
        JSR sub         ; JSR at $4003: pushes $40 then $05 (PC+2)
        STA $8000
        BRK
sub     LDA $01FF       ; $42
        CLC
        ADC $01FE       ; + $40 = $82
        ADC $01FD       ; + $05 = $87
        RTS
