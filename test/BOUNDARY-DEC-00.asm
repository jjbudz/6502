; Test DEC on $00 Boundary
; Tests: DEC $00 wraps to $FF, sets N=1, Z=0
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$00
        STAZ $10
        DECZ $10      ; $00 - 1 wraps to $FF
        BMI ok1       ; N must be set
        BRK
ok1     BNE ok2       ; Z must be clear
        BRK
ok2     LDAZ $10
        CMPI #$FF     ; Memory must actually hold $FF
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
