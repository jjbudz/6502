; With I set, timer IRQs (run with --irq-every 10) are held off: the
; handler never runs during a 50-iteration loop.
;   $8000 = $01  main code finished
;   $8001 = $00  handler never ran
        .ORG $4000
        SEI
        LDX #50
loop    DEX
        BNE loop
        LDA #$01
        STA $8000
done    JMP done            ; I is set, so this halts

handler INC $8001
        RTI

        .ORG $FFFE
        .WORD handler
