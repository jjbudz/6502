; Test ADC Flag Interactions - Negative, Overflow, Zero
; Tests: $40 + $40 = $80 sets N=1, V=1, Z=0 simultaneously
; Result stored at $8000 should be $01 (success)
$4000   CLC
        LDAI #$40
        ADCI #$40     ; $80: N=1 V=1 Z=0
        BMI ok1       ; N must be set
        BRK
ok1     BVS ok2       ; V must be set
        BRK
ok2     BNE ok3       ; Z must be clear
        BRK
ok3     CMPI #$80
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
