; Test SBC (Subtract with Carry) - Immediate addressing mode
; Tests: SBC #$imm
; Expected: $80 - $30 - (1-carry) = $80 - $30 = $50
; With carry set (no borrow), result is $50
; Result stored at $8000 should be $50
$4000   SEC
        LDAI #$80
        SBCI #$30
        STAA $8000
        BRK
