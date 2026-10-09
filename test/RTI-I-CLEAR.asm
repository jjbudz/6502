; RTI must restore the interrupt-disable flag (issue #60). I starts at 1,
; RTI pulls $00, so I must end at 0. The test checks I=0 in the -pf flag dump.
        .ORG $4000
        SEI
        LDA #>resume
        PHA
        LDA #<resume
        PHA
        LDA #$00
        PHA
        RTI
        BRK             ; not reached
resume
        LDA #$01
        STA $8000
        BRK
