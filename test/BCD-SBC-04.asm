; Test BCD SBC with borrow in
; Tests: SED; CLC (borrow in); $50 - $25 - 1 = $24 decimally, no borrow out
; Expected: A=$24, C=1
; Result stored at $8000 should be $01 (success)
$4000   SED
        CLC           ; Borrow in
        LDAI #$50
        SBCI #$25
        BCS ok1       ; C must be set (no borrow out)
        BRK
ok1     CMPI #$24
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
