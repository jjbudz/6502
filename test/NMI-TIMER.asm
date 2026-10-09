; Timer NMIs (run with --nmi-every 100) are taken even with I set, through
; the vector at $FFFA. The handler counts them; main code waits for three.
; With no IRQ/BRK handler installed, the final BRK ends the run (a jump to
; self would not, since another NMI can always arrive).
;   $8000 = $01  main code finished
;   $8001 = $03  handler ran three times
        .ORG $4000
        SEI
wait    LDA $8001
        CMP #$03
        BNE wait
        LDA #$01
        STA $8000
        BRK

nmi     INC $8001
        RTI

        .ORG $FFFA
        .WORD nmi
