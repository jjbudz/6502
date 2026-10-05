; Test INC on $FF Boundary
; Tests: INC $FF wraps to $00, sets Z=1, N=0
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$FF
        STAZ $10
        INCZ $10      ; $FF + 1 wraps to $00
        BEQ ok1       ; Z must be set
        BRK
ok1     BPL ok2       ; N must be clear
        BRK
ok2     LDAZ $10
        BEQ pass      ; Memory must actually hold $00
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
