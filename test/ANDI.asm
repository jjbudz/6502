; Test AND (Bitwise AND) - Immediate addressing mode
; Tests: AND #$imm
; Expected: $FF AND $55 = $55
; Result stored at $8000 should be $55
$4000   LDAI #$FF
        ANDI #$55
        STAA $8000
        BRK
