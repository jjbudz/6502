; Test Stack Wraparound - Push from $0100
; Tests: Stack pointer wraps from $00 to $FF when pushing
; Expected: After pushing with SP=$00, SP becomes $FF (wraps to top of stack)
$4000   LDXI #$00     ; Set stack pointer to $00
        TXS
        LDAI #$42     ; Value to push
        PHA           ; Push, SP wraps $00→$FF
        TSX           ; Get SP into X
        STXA $8000    ; Should be $FF
        BRK
