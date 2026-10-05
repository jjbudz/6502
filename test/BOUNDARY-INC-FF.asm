; Test INC on $FF Boundary
; Tests: INC $FF wraps to $00 and sets zero flag
; Expected: $FF + 1 = $00, Z flag set, N flag clear
$4000   LDAI #$FF
        STAZ $10
        INCZ $10      ; $FF + 1 = $00
        BEQ $400C     ; Should branch (zero)
        BRK           ; Shouldn't reach
$400C   BMI $4010     ; Should not branch (not negative)
        BRK           ; Shouldn't reach
$4010   LDAZ $10
        STAA $8000    ; Should be $00
        BRK
