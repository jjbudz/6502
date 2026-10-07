; Test Overflow Flag - Positive + Positive = Negative
; Tests: $50 + $50 = $A0 sets V=1, N=1, C=0
; Result stored at $8000 should be $01 (success)
$4000   CLC
        LDAI #$50     ; +80
        ADCI #$50     ; +80 -> $A0 (-96 signed): overflow
        BVS ok1       ; V must be set
        BRK
ok1     BMI ok2       ; N must be set
        BRK
ok2     BCC ok3       ; C must be clear (no unsigned carry)
        BRK
ok3     CMPI #$A0
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
