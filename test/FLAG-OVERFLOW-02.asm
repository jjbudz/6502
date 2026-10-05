; Test Overflow Flag - Negative + Negative = Positive
; Tests: Overflow when two negative numbers sum to positive
; Expected: $90 + $90 = $20 with carry (overflow, positive result)
$4000   CLC
        LDAI #$90     ; Negative (-112 decimal)
        ADCI #$90     ; Negative (-112 decimal)
        BVS $400C     ; Should branch (overflow)
        BRK           ; Shouldn't reach
$400C   BPL $4010     ; Should branch (positive result)
        BRK           ; Shouldn't reach
$4010   BCS $4014     ; Should branch (carry set)
        BRK           ; Shouldn't reach
$4014   LDAI #$01
        STAA $8000    ; Success
        BRK
