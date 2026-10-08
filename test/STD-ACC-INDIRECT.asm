; Standard syntax: accumulator mode (LSR A, ASL A), JMP (indirect),
; and zero page INC/DEC.
$4000   LDA #$10
        ASL A            ; $20
        ASL A            ; $40
        LSR A            ; $20
        STA $50
        INC $50          ; $21
        INC $50          ; $22
        DEC $50          ; $21
        LDA #$20         ; pointer at $10/$11 -> $4020
        STA $10
        LDA #$40
        STA $11
        JMP ($0010)
$4018   LDA #$FF         ; not reached
        STA $8000
        BRK
$4020   LDA $50
        STA $8000
        BRK
