; Test DEC on $00 Boundary
; Tests: DEC $00 wraps to $FF and sets negative flag
; Expected: $00 - 1 = $FF, N flag set, Z flag clear
$4000   LDAI #$00
        STAZ $10
        DECZ $10      ; $00 - 1 = $FF
        BMI $400C     ; Should branch (negative)
        BRK           ; Shouldn't reach
$400C   BEQ $4010     ; Should not branch (not zero)
        BRK           ; Shouldn't reach
$4010   LDAZ $10
        STAA $8000    ; Should be $FF
        BRK
