; < and > take the low and high byte of a value, here of a label defined
; later in the file. Builds a pointer to TARGET and jumps through it.
        .ORG $4000
        LDA #<TARGET
        STA $10
        LDA #>TARGET
        STA $11
        JMP ($0010)
        LDA #$FF           ; not reached
        STA $8000
        BRK
TARGET  LDA #>TARGET       ; $40
        CLC
        ADC #<TARGET       ; + $11 (TARGET is at $4011)
        STA $8000
        BRK
