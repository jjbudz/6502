; The program sets the 6502 reset vector at $FFFC/$FFFD, and the emulator
; starts there when -r is given without an address.
        * = $4000
        LDA #$FF           ; not the entry point
        STA $8000
        BRK

start:  LDA #$01
        STA $8000
        BRK

        * = $FFFC
        .WORD start
