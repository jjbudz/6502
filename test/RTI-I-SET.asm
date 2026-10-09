; RTI must restore the interrupt-disable flag (issue #60). I starts at 0,
; RTI pulls $04, so I must end at 1. The test checks I=1 in the -pf flag dump.
        .ORG $4000
        CLI
        LDA #>resume
        PHA
        LDA #<resume
        PHA
        LDA #$04
        PHA
        RTI
        BRK             ; not reached
resume
        LDA #$01
        STA $8000
        BRK
