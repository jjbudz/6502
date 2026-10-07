; Test BCD ADC NMOS flag quirk - large sum
; Tests: SED; $99 + $99 = 198 -> A=$98 with carry. On NMOS the
;        intermediate sum is $138: bit 7 of $38 is clear so N=0, and the
;        signed sum -112 + -112 + 24 underflows so V=1
; Expected: A=$98, N=0, V=1, C=1
; Result stored at $8000 should be $01 (success)
$4000   SED
        CLC
        LDAI #$99
        ADCI #$99
        BPL ok1       ; N must be clear
        BRK
ok1     BVS ok2       ; V must be set
        BRK
ok2     BCS ok3       ; C must be set
        BRK
ok3     CMPI #$98
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
