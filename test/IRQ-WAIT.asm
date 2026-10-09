; A jump to self with I clear and timer IRQs running (--irq-every 50) is a
; wait for an interrupt, not the end of the program. On the third IRQ the
; handler sets I in the pushed P, so after RTI the wait loop halts.
;   $8001 = $03  handler ran three times
; The test also checks the run stops at the wait loop ($4001).
        .ORG $4000
        CLI
wait    JMP wait

handler INC $8001
        LDA $8001
        CMP #$03
        BNE ret
        TSX
        LDA $0101,X         ; P as pushed by the IRQ
        ORA #$04            ; return with I set
        STA $0101,X
ret     RTI

        .ORG $FFFE
        .WORD handler
