; Timer IRQs (run with --irq-every 100) are taken while I is clear. The
; handler counts them; main code waits for three, then masks IRQs and ends.
;   $8000 = $01  main code finished
;   $8001 = $03  handler ran three times
;   $8002 = $20  P pushed by an IRQ has B clear and bit 5 set
        .ORG $4000
        CLI
wait    LDA $8001
        CMP #$03
        BNE wait
        SEI                 ; no more IRQs, so the jump to self halts
        LDA #$01
        STA $8000
done    JMP done

handler INC $8001
        TSX
        LDA $0101,X         ; P as pushed by the IRQ
        AND #$30
        STA $8002
        RTI

        .ORG $FFFE
        .WORD handler
