; Test BCD SBC NMOS flags - all flags come from the binary subtraction
; Tests: SED; SEC; $80 - $01 = $79 decimally. On NMOS the flags are those of
;        the binary result $7F: C=1 (no borrow), V=1 (signed overflow), N=0
; Expected: A=$79, C=1, V=1, N=0
; Result stored at $8000 should be $01 (success)
$4000   SED
        SEC
        LDAI #$80
        SBCI #$01
        BCS ok1       ; C must be set (no borrow)
        BRK
ok1     BVS ok2       ; V must be set (binary $80-$01 overflows)
        BRK
ok2     BPL ok3       ; N must be clear (binary result $7F)
        BRK
ok3     CMPI #$79
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
