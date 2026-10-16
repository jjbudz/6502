; CPY sets carry when Y >= the operand, in all three addressing modes.
; The old code took carry from bit 7 of the difference, wrong for most of
; these cases. Each compare starts with carry set to the wrong value.
; Stores $01 at $8000 on success, $00 on failure.
        .ORG $4000
        ; Y=$00 vs $00: C=1
        LDY #$00
        LDA #$00
        STA $10
        STA $0300
        CLC
        CPY #$00
        BCS *+5
        JMP fail
        CLC
        CPY $10
        BCS *+5
        JMP fail
        CLC
        CPY $0300
        BCS *+5
        JMP fail
        ; Y=$20 vs $10: C=1
        LDY #$20
        LDA #$10
        STA $10
        STA $0300
        CLC
        CPY #$10
        BCS *+5
        JMP fail
        CLC
        CPY $10
        BCS *+5
        JMP fail
        CLC
        CPY $0300
        BCS *+5
        JMP fail
        ; Y=$10 vs $20: C=0
        LDY #$10
        LDA #$20
        STA $10
        STA $0300
        SEC
        CPY #$20
        BCC *+5
        JMP fail
        SEC
        CPY $10
        BCC *+5
        JMP fail
        SEC
        CPY $0300
        BCC *+5
        JMP fail
        ; Y=$90 vs $10: C=1
        LDY #$90
        LDA #$10
        STA $10
        STA $0300
        CLC
        CPY #$10
        BCS *+5
        JMP fail
        CLC
        CPY $10
        BCS *+5
        JMP fail
        CLC
        CPY $0300
        BCS *+5
        JMP fail
        ; Y=$05 vs $FF: C=0
        LDY #$05
        LDA #$FF
        STA $10
        STA $0300
        SEC
        CPY #$FF
        BCC *+5
        JMP fail
        SEC
        CPY $10
        BCC *+5
        JMP fail
        SEC
        CPY $0300
        BCC *+5
        JMP fail
        LDA #$01
        STA $8000
        BRK
fail    LDA #$00
        STA $8000
        BRK
