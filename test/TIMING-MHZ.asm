; Throttle accuracy at a realistic clock rate: about 1.3 million cycles of
; nested countdown loops, which should take about 1.3 seconds at 1 MHz.
; Sleeping after each 2-7 cycle instruction cannot hold this rate; the test
; checks wall time against the cycle count the emulator reports.
* = $4000
        LDA #$04
        STA $10         ; outer passes
outer   LDY #$00        ; 256 middle passes
middle  LDX #$00        ; 256 inner passes
inner   DEX
        BNE inner
        DEY
        BNE middle
        DEC $10
        BNE outer
        LDA #$01
        STA $8000
        BRK
