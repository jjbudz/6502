; Test BCD Subtraction - Borrow generation
; Tests: SBC in decimal mode with borrow
; Expected: $32 - $35 = $97 in BCD (borrow from hundreds)
; Result stored at $8000 should be $97, carry clear (borrow occurred)
$4000   SED
        SEC           ; Set carry (no initial borrow)
        LDAI #$32
        SBCI #$35
        BCS fail      ; Carry should be clear (borrow occurred)
        CMPI #$97     ; 32 - 35 = -3 = 97 in BCD with borrow
        BNE fail2
        STAA $8000
        CLD
        BRK
fail    LDAI #$FE
        STAA $8000
        CLD
        BRK
fail2   LDAI #$FD
        STAA $8000
        CLD
        BRK
