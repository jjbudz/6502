; Test ASL on $80 - Shift Out Sign Bit
; Tests: ASL $80 shifts out bit 7 into carry
; Expected: $80 << 1 = $00, C flag set, Z flag set, N flag clear
$4000   LDAI #$80
        ASL           ; Shift left: $80 → $00, carry set
        BCS $4008     ; Should branch (carry set)
        BRK           ; Shouldn't reach
$4008   BEQ $400C     ; Should branch (zero)
        BRK           ; Shouldn't reach
$400C   BMI $4010     ; Should not branch (not negative)
        BRK           ; Shouldn't reach
$4010   STAA $8000    ; Should be $00
        BRK
