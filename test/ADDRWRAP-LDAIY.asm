; LDA ($80),Y with pointer $FFF0 and Y=$20 must wrap to $0010 (16-bit
; address bus), not index past the end of 64K memory.
$4000   LDAI #$F0
        STAZ $80
        LDAI #$FF
        STAZ $81
        LDAI #$5A
        STAZ $10
        LDYI #$20
        LDAI #$00
        LDAIY $80
        STAA $8000
        BRK
