; Test ROL on $FF with Carry In
; Tests: ROL A with $FF and C=1 gives $FF, sets C=1, N=1
; Result stored at $8000 should be $01 (success)
$4000   SEC           ; Carry in
        LDAI #$FF
        ROL           ; Bit 7 out to carry, carry in to bit 0 -> $FF
        BCS ok1       ; C must be set
        BRK
ok1     BMI ok2       ; N must be set
        BRK
ok2     CMPI #$FF
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
