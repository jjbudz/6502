; Test ADC Sign Bit Transition at $7F
; Tests: Adding to $7F causes sign bit change and overflow
; Expected: $7F + $01 = $80 (positive→negative transition, overflow)
$4000   CLC
        LDAI #$7F     ; Maximum positive signed byte
        ADCI #$01     ; Add 1
        BVS $400C     ; Should branch (overflow occurred)
        BRK           ; Shouldn't reach
$400C   BMI $4010     ; Should branch (result is negative)
        BRK           ; Shouldn't reach
$4010   CMPI #$80
        BEQ $4018     ; Should branch
        BRK           ; Shouldn't reach
$4018   LDAI #$01
        STAA $8000    ; Success
        BRK
