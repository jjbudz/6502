; Test SBC Borrow Handling - Carry acts as inverted borrow
; Tests: SBC with carry set (no borrow) and carry clear (borrow in)
; Result stored at $8000 should be $01 (success)
$4000   SEC           ; No borrow
        LDAI #$50
        SBCI #$30     ; $50 - $30 = $20, C stays 1
        BCS ok1
        BRK
ok1     CMPI #$20
        BEQ ok2
        BRK
ok2     CLC           ; Borrow in
        LDAI #$50
        SBCI #$30     ; $50 - $30 - 1 = $1F, no borrow out (C=1)
        BCS ok3
        BRK
ok3     CMPI #$1F
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
