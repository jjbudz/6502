; Test ASL (Arithmetic Shift Left) - Accumulator addressing mode
; Tests: ASL A
; Expected: $55 << 1 = $AA (01010101 -> 10101010)
; Sets carry flag to 0 (bit 7 was 0)
; Result stored at $8000 should be $AA
$4000   LDAI #$55
        ASL
        STAA $8000
        BRK
