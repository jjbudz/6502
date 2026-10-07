; Test SEI (Set Interrupt Disable)
; Tests: SEI sets the I flag (bit 2 of P), observed via PHP/PLA
; Result stored at $8000 should be $01 (success)
$4000   CLI           ; Start with I clear
        SEI           ; Set I
        PHP           ; Push P
        PLA           ; A = P
        ANDI #$04     ; Isolate I flag
        CMPI #$04
        BNE fail
        LDAI #$01
        STAA $8000
        BRK
fail    BRK
