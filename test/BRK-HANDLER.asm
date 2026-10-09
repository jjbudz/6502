; BRK with a handler installed at $FFFE: it pushes the address past its
; padding byte and P with B set, sets I, and jumps through the vector; RTI
; resumes past the padding byte with I restored. The padding byte is $00,
; so returning onto it would run the handler a second time.
;   $8000 = $01  main code resumed after the handler
;   $8001 = $14  handler saw B ($10) in the pushed P and I ($04) set
;   $8002 = $00  I clear again after RTI
;   $8003 = $01  handler ran exactly once
        .ORG $4000
        CLI
        BRK
        .BYTE $00           ; padding byte, skipped by RTI
        PHP
        PLA
        AND #$04
        STA $8002
        LDA #$01
        STA $8000
done    JMP done            ; ends the run (a bare BRK would re-enter)

handler INC $8003
        TSX
        LDA $0101,X         ; P as pushed by BRK
        AND #$10
        STA $8001
        PHP
        PLA
        AND #$04            ; I inside the handler
        ORA $8001
        STA $8001
        RTI

        .ORG $FFFE
        .WORD handler
