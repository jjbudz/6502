; Test SBC Sign Bit Transition at $80
; Tests: $80 - $01 = $7F sets V=1, N=0, C=1
; Result stored at $8000 should be $01 (success)
$4000   SEC           ; No borrow
        LDAI #$80     ; Smallest negative signed byte
        SBCI #$01     ; -> $7F: negative to positive = overflow
        BVS ok1       ; V must be set
        BRK
ok1     BPL ok2       ; N must be clear
        BRK
ok2     BCS ok3       ; C must remain set (no borrow)
        BRK
ok3     CMPI #$7F
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
