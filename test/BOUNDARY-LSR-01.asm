; Test LSR on $01 - Shift Out Least Significant Bit
; Tests: LSR $01 shifts bit 0 into carry
; Expected: $01 >> 1 = $00, C flag set, Z flag set
$4000   LDAI #$01
        LSR           ; Shift right: $01 → $00, carry set
        BCS $4008     ; Should branch (carry set)
        BRK           ; Shouldn't reach
$4008   BEQ $400C     ; Should branch (zero)
        BRK           ; Shouldn't reach
$400C   STAA $8000    ; Should be $00
        BRK
