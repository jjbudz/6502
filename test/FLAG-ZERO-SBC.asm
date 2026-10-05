; Test Zero Flag - SBC Producing Zero
; Tests: Zero flag set when SBC produces $00 result
; Expected: $42 - $42 = $00 with no borrow, Z flag set
$4000   SEC           ; Set carry (no borrow)
        LDAI #$42
        SBCI #$42     ; $42 - $42 = $00
        BEQ $400C     ; Should branch (zero)
        BRK           ; Shouldn't reach
$400C   BCS $4010     ; Should branch (no borrow)
        BRK           ; Shouldn't reach
$4010   STAA $8000    ; Should be $00
        BRK
