; STA ($80),Y with pointer $FFF0 and Y=$20 must wrap to $0010 (16-bit
; address bus), not index past the end of 64K memory.
$4000   LDAI #$F0
        STAZ $80
        LDAI #$FF
        STAZ $81
        LDYI #$20
        LDAI #$5A
        STAIY $80
        LDAZ $10
        STAA $8000
        BRK
