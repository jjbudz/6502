; * is the address of the current instruction. JMP *+6 skips the three
; bytes after the three-byte JMP itself.
        .ORG $4000
        LDA #1
        JMP *+6            ; at $4002, so the target is $4008
        LDA #$FF           ; $4005, skipped
        BRK                ; $4007, skipped
        STA $8000          ; $4008
        BRK
