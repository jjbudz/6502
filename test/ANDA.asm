; Test AND (Bitwise AND) - Absolute addressing mode
; Tests: AND $addr
; Expected: $FF AND $55 = $55
; Result stored at $8000 should be $55
$4000   LDAI #$55
        STAA $8100
        LDAI #$FF
        ANDA $8100
        STAA $8000
        BRK
