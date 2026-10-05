; Test SBC Sign Bit Transition at $80
; Tests: Subtracting from $80 causes overflow
; Expected: $80 - $01 = $7F (negative→positive transition, overflow)
$4000   SEC           ; Set carry (no borrow)
        LDAI #$80     ; Minimum negative signed byte
        SBCI #$01     ; Subtract 1
        BVS $400C     ; Should branch (overflow occurred)
        BRK           ; Shouldn't reach
$400C   BPL $4010     ; Should branch (result is positive)
        BRK           ; Shouldn't reach
$4010   CMPI #$7F
        BEQ $4018     ; Should branch
        BRK           ; Shouldn't reach
$4018   LDAI #$01
        STAA $8000    ; Success
        BRK
