; CPX sets carry when X >= the operand, in all three addressing modes.
; The old code took carry from bit 7 of the difference, wrong for most of
; these cases. Each compare starts with carry set to the wrong value.
; Stores $01 at $8000 on success, $00 on failure.
        .ORG $4000
        ; X=$00 vs $00: C=1
        LDX #$00
        LDA #$00
        STA $10
        STA $0300
        CLC
        CPX #$00
        BCS *+5
        JMP fail
        CLC
        CPX $10
        BCS *+5
        JMP fail
        CLC
        CPX $0300
        BCS *+5
        JMP fail
        ; X=$20 vs $10: C=1
        LDX #$20
        LDA #$10
        STA $10
        STA $0300
        CLC
        CPX #$10
        BCS *+5
        JMP fail
        CLC
        CPX $10
        BCS *+5
        JMP fail
        CLC
        CPX $0300
        BCS *+5
        JMP fail
        ; X=$10 vs $20: C=0
        LDX #$10
        LDA #$20
        STA $10
        STA $0300
        SEC
        CPX #$20
        BCC *+5
        JMP fail
        SEC
        CPX $10
        BCC *+5
        JMP fail
        SEC
        CPX $0300
        BCC *+5
        JMP fail
        ; X=$90 vs $10: C=1
        LDX #$90
        LDA #$10
        STA $10
        STA $0300
        CLC
        CPX #$10
        BCS *+5
        JMP fail
        CLC
        CPX $10
        BCS *+5
        JMP fail
        CLC
        CPX $0300
        BCS *+5
        JMP fail
        ; X=$05 vs $FF: C=0
        LDX #$05
        LDA #$FF
        STA $10
        STA $0300
        SEC
        CPX #$FF
        BCC *+5
        JMP fail
        SEC
        CPX $10
        BCC *+5
        JMP fail
        SEC
        CPX $0300
        BCC *+5
        JMP fail
        LDA #$01
        STA $8000
        BRK
fail    LDA #$00
        STA $8000
        BRK
