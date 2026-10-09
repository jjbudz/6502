; .TEXT stores the characters of a string, case preserved, and a plain
; expression argument adds a byte (here a terminating 0).
        .ORG $4000
        LDA MSG            ; 'H' = $48
        CLC
        ADC MSG+1          ; 'i' = $69 -> $B1
        ADC MSG+2          ; 0
        STA $8000
        BRK
MSG     .TEXT "Hi", 0
