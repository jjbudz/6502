; Test Zero Flag - ADC Producing Zero
; Tests: Zero flag set when ADC produces $00 result
; Expected: $FF + $01 = $00 with carry, Z flag set
$4000   CLC
        LDAI #$FF
        ADCI #$01     ; $FF + $01 = $00 with carry
        BEQ $400C     ; Should branch (zero)
        BRK           ; Shouldn't reach
$400C   BCS $4010     ; Should branch (carry set)
        BRK           ; Shouldn't reach
$4010   STAA $8000    ; Should be $00
        BRK
