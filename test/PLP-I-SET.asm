; PLP must restore the interrupt-disable flag (issue #60). I starts at 0,
; PLP pulls $04, so I must end at 1. The test checks I=1 in the -pf flag dump.
        .ORG $4000
        CLI
        LDA #$04
        PHA
        PLP
        LDA #$01
        STA $8000
        BRK
