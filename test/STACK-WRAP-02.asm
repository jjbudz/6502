; Test Stack Wraparound - Pull at $FF
; Tests: Stack pointer wraps from $FF to $00 when pulling
; Expected: After pulling with SP=$FF, SP becomes $00
$4000   LDXI #$FF     ; Set stack pointer to $FF
        TXS
        LDAI #$99
        STAA $01FF    ; Store value at top of stack
        PLA           ; Pull, SP wraps $FF→$00
        TSX           ; Get SP into X
        STXA $8000    ; Should be $00
        BRK
