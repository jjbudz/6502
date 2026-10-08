; .ORG sets the location; .BYTE and .WORD lay out data that code refers to
; by label. TABLE and PTR are forward references, so they are absolute.
        .ORG $4000
        LDX #2
        LDA TABLE,X        ; 3
        CLC
        ADC PTR            ; low byte of $1234 = $34 -> $37
        ADC PTR+1          ; high byte      = $12 -> $49
        STA $8000
        BRK
TABLE   .BYTE 1, 2, 3
PTR     .WORD $1234
