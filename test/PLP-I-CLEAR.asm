; PLP must restore the interrupt-disable flag (issue #60). I starts at 1,
; PLP pulls $00, so I must end at 0. The test checks I=0 in the -pf flag dump.
        .ORG $4000
        SEI
        LDA #$00
        PHA
        PLP
        LDA #$01
        STA $8000
        BRK
