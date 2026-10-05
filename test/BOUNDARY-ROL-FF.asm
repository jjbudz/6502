; Test ROL on $FF with Carry
; Tests: ROL $FF with carry set rotates to $FF (all bits shift, carry in)
; Expected: C:$FF → $FF:C, result $FF, carry set
$4000   SEC           ; Set carry
        LDAI #$FF
        ROL           ; Rotate left: $FF with C → $FF, carry set
        BCS $4008     ; Should branch (carry set)
        BRK           ; Shouldn't reach
$4008   CMPI #$FF
        BEQ $4010     ; Should branch (still $FF)
        BRK           ; Shouldn't reach
$4010   STAA $8000    ; Should be $FF
        BRK
