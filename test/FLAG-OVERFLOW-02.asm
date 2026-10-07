; Test Overflow Flag - Negative + Negative = Positive
; Tests: $90 + $90 = $20 with carry sets V=1, N=0, C=1
; Result stored at $8000 should be $01 (success)
$4000   CLC
        LDAI #$90     ; -112
        ADCI #$90     ; -112 -> $120 truncates to $20: overflow + carry
        BVS ok1       ; V must be set
        BRK
ok1     BPL ok2       ; N must be clear
        BRK
ok2     BCS ok3       ; C must be set
        BRK
ok3     CMPI #$20
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
