; Test Overflow Flag - Positive + Positive = Negative
; Tests: Classic overflow case: two positive numbers sum to negative
; Expected: $50 + $50 = $A0 (overflow, negative result)
$4000   CLC
        LDAI #$50     ; Positive (80 decimal)
        ADCI #$50     ; Positive (80 decimal)
        BVS $400C     ; Should branch (overflow)
        BRK           ; Shouldn't reach
$400C   BMI $4010     ; Should branch (negative result)
        BRK           ; Shouldn't reach
$4010   CMPI #$A0
        BEQ $4018     ; Should branch
        BRK           ; Shouldn't reach
$4018   LDAI #$01
        STAA $8000    ; Success
        BRK
