; Test CLI (Clear Interrupt Disable)
; Tests: CLI clears the I flag (bit 2 of P), observed via PHP/PLA
; Result stored at $8000 should be $01 (success)
$4000   SEI           ; Start with I set
        CLI           ; Clear I
        PHP           ; Push P
        PLA           ; A = P
        ANDI #$04     ; Isolate I flag
        BNE fail      ; Must be zero
        LDAI #$01
        STAA $8000
        BRK
fail    BRK
