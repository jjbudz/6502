; Expressions: + and - over constants and labels, including a forward
; label in an immediate (TABLE+1-TABLE = 1, known only in pass 2).
BASE = $50
        .ORG $4000
        LDA #3
        STA BASE+2         ; zero page $52: BASE is a known constant
        LDA #4
        STA BASE+3         ; $53
        LDA BASE+2         ; 3
        CLC
        ADC BASE+3         ; 7
        ADC #10-3          ; 14
        ADC #TABLE+1-TABLE ; 15 = $0F
        STA $8000
        BRK
TABLE   .BYTE 0
