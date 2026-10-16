; BIT copies bits 7 and 6 of the operand into N and V, and sets Z from
; A AND operand. The old code took N from A AND operand, wrong whenever A
; lacks bit 7 but the operand has it. Checks zero page and absolute forms.
; Stores $01 at $8000 on success, $00 on failure.
        .ORG $4000
        ; A=$01 M=$82: N=1 V=0 Z=1
        LDA #$82
        STA $10
        STA $0300
        LDA #$01
        BIT $10
        BMI *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        LDA #$01
        BIT $0300
        BMI *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        ; A=$01 M=$80: N=1 V=0 Z=1
        LDA #$80
        STA $10
        STA $0300
        LDA #$01
        BIT $10
        BMI *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        LDA #$01
        BIT $0300
        BMI *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        ; A=$FF M=$41: N=0 V=1 Z=0
        LDA #$41
        STA $10
        STA $0300
        LDA #$FF
        BIT $10
        BPL *+5
        JMP fail
        BVS *+5
        JMP fail
        BNE *+5
        JMP fail
        LDA #$FF
        BIT $0300
        BPL *+5
        JMP fail
        BVS *+5
        JMP fail
        BNE *+5
        JMP fail
        ; A=$80 M=$00: N=0 V=0 Z=1
        LDA #$00
        STA $10
        STA $0300
        LDA #$80
        BIT $10
        BPL *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        LDA #$80
        BIT $0300
        BPL *+5
        JMP fail
        BVC *+5
        JMP fail
        BEQ *+5
        JMP fail
        ; A=$C0 M=$C0: N=1 V=1 Z=0
        LDA #$C0
        STA $10
        STA $0300
        LDA #$C0
        BIT $10
        BMI *+5
        JMP fail
        BVS *+5
        JMP fail
        BNE *+5
        JMP fail
        LDA #$C0
        BIT $0300
        BMI *+5
        JMP fail
        BVS *+5
        JMP fail
        BNE *+5
        JMP fail
        LDA #$01
        STA $8000
        BRK
fail    LDA #$00
        STA $8000
        BRK
